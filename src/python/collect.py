from selenium import webdriver
from selenium.common.exceptions import WebDriverException, StaleElementReferenceException, NoSuchElementException
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import time

def _get_driver():
    args = [] # ["--headless"]

    def add_args(options):
        for arg in args:
            options.add_argument(arg)

    try:
        options = webdriver.FirefoxOptions()
        add_args(options)
        return webdriver.Firefox(options=options)
    except WebDriverException:
        pass

    try:
        options = webdriver.ChromeOptions()
        add_args(options)
        return webdriver.Chrome(options=options)
    except WebDriverException:
        pass

    try:
        options = webdriver.EdgeOptions()
        add_args(options)
        return webdriver.Edge(options=options)
    except WebDriverException:
        pass

    try:
        options = webdriver.IeOptions()
        add_args(options)
        return webdriver.Ie(options=options)
    except WebDriverException:
        pass

    try:
        options = webdriver.SafariOptions()
        add_args(options)
        return webdriver.Safari(options=options)
    except WebDriverException:
        pass

    raise RuntimeError("[collect.py] When trying to collect songs: no supported browser was found on your system (Firefox, Chrome, Edge, Ie, Safari)")

def _progress_counter(i: int, dt: int):
    throbber = [".  ", ".. ", "...", " ..", "  .", "   "]
    print(f" {throbber[dt%len(throbber)]} [{i} songs processed]", end="\r")

class SongData:
    def __init__(self, title: str, artist: str) -> None:
        self.title = title
        self.artist = artist

    def __str__(self) -> str:
        return self._csv_escape(self.title) + "," + self._csv_escape(self.artist)

    def _csv_escape(self, s: str) -> str:
        return '"' + s.replace('"', '""') + '"'

    def _key(self):
        return (self.title, self.artist)

    def __eq__(self, other):
        return isinstance(other, SongData) and self._key() == other._key()

    def __hash__(self):
        return hash(self._key())

def from_spotify(query: str) -> list[SongData]:
    song_container_css_path = '[data-testid="tracklist-row"] div[role="gridcell"][aria-colindex="2"]'
    title_css_path = '[data-testid="internal-track-link"] div'
    artist_css_path = 'a[href*="/artist/"]'

    # Correct the URL based on the query
    url = query
    if not "open.spotify.com" in url:
        # Assuming the id of the playlist was given
        url = "https://open.spotify.com/playlist/" + query

    if not url.startswith("https://"):
        url = "https://" + url

    results = []

    driver = _get_driver()
    driver.get(url)

    # Scroll webpage and gather songs

    last_bottom_song = None

    def wait_and_collect_songs(timeout: float = 10) -> list:
        # Wait for the songs to load
        test_song_container = WebDriverWait(driver, timeout).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, song_container_css_path))
        )

        loop_sleep_amount = 0.1
        has_loaded = False
        i = 0
        while not has_loaded: # Loop until the song loads
            if i > timeout / loop_sleep_amount:
                raise TimeoutError(f"Failed to collect songs from playlist {url}.")

            try:
                if test_song_container.find_element(By.CSS_SELECTOR, title_css_path).text:
                    has_loaded = True
            except StaleElementReferenceException:
                has_loaded = True

            time.sleep(loop_sleep_amount)
            i += 1

        # Collect songs
        all_songs = driver.find_elements(By.CSS_SELECTOR, song_container_css_path)
        for song_container in all_songs:
            try:
                title = song_container.find_element(By.CSS_SELECTOR, title_css_path).text
                artist = song_container.find_element(By.CSS_SELECTOR, artist_css_path).text
            except StaleElementReferenceException:
                continue
            except NoSuchElementException:
                return []

            song_data = SongData(title, artist)
            if song_data not in results:
                results.append(song_data)

        return all_songs

    dt = 0
    while True:
        all_songs = wait_and_collect_songs()

        _progress_counter(len(results), dt)

        if (len(all_songs) == 0) or (last_bottom_song == results[-1]):
            break

        last_bottom_song = results[-1]

        # Scroll down
        driver.execute_script("arguments[0].scrollIntoView(true);", all_songs[-1])

        dt += 1

    wait_and_collect_songs() # Last batch

    return results

def from_ytmusic(query: str) -> list[SongData]:
    reject_button_css_path = '[action="https://consent.youtube.com/save"]'
    content_css_path = 'body ytmusic-app ytmusic-app-layout#layout.style-scope.ytmusic-app div#content.style-scope.ytmusic-app ytmusic-browse-response#browse-page.style-scope.ytmusic-app div.background-gradient.style-scope.ytmusic-browse-response div#content-wrapper.style-scope.ytmusic-browse-response div#contents.style-scope.ytmusic-browse-response ytmusic-two-column-browse-results-renderer.style-scope.ytmusic-browse-response div#secondary.style-scope.ytmusic-two-column-browse-results-renderer ytmusic-section-list-renderer.description.scroller.scroller-on-hover.style-scope.ytmusic-two-column-browse-results-renderer div#contents.style-scope.ytmusic-section-list-renderer ytmusic-playlist-shelf-renderer.style-scope.ytmusic-section-list-renderer.fullbleed div.style-scope.ytmusic-playlist-shelf-renderer div#contents.style-scope.ytmusic-playlist-shelf-renderer'

    url = query
    if not "music.youtube.com" in query:
        url = "https://music.youtube.com/playlist?list=" + query

    if not url.startswith("https://"):
        url = "https://" + url

    results = []

    driver = _get_driver()
    driver.get(url)

    WebDriverWait(driver, 5).until(
        EC.presence_of_element_located((By.CSS_SELECTOR, reject_button_css_path))
    ).submit()

    content = WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.CSS_SELECTOR, content_css_path))
    )

    for div in content.find_elements(By.CSS_SELECTOR, "ytmusic-responsive-list-item-renderer"):
        song_container = div.find_element(By.CSS_SELECTOR, 'div[class="flex-columns style-scope ytmusic-responsive-list-item-renderer"]')

        title = song_container.find_elements(By.CSS_SELECTOR, 'yt-formatted-string')[0].text
        artist = song_container.find_elements(By.CSS_SELECTOR, 'yt-formatted-string')[1].text

        song_data = SongData(title, artist)
        if not song_data in results:
            results.append(song_data)

    return results

# r = from_spotify("2bYs7IlWQqyuLHmcd4bt9g")
r = from_ytmusic("PLC7BP2QZenrVUrZPciCuqma4JpL8mtP_B")
print("\n".join([str(i) for i in r]))
print(len(r))

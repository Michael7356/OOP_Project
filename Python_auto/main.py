import base64
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support.ui import Select
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.support import expected_conditions as EC
import time
import ddddocr
import os
import sys
import json

def load_config():
    base_path = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(base_path, "..", "cmake-build-debug", "config.json")

    with open(config_path, "r", encoding='utf-8') as f:
        config = json.load(f)
    return config

def wait_for_download(driver, timeout= 30):
    driver.get("chrome://downloads/")
    end_time = time.time() + timeout
    while True:
        try:
            progress = driver.execute_script("""
                var items = document.querySelector('downloads-manager').shadowRoot.querySelectorAll('downloads-item');
                if (items.length === 0) return 0;
                return items[0].shadowRoot.querySelector('#progress').value;
            """)
            if progress == 100:
                return True
        except:
            pass
        if time.time() > end_time:
            return False
        time.sleep(1)

config = load_config()
Account = config["Account"]
Password = config["Password"]
chrome_options = Options()
# chrome_options.add_argument("--headless")
chrome_options.add_argument("--window-size=1920,1080")
chrome_options.add_experimental_option("detach", True)

download_dir = os.path.join(os.getcwd(), "downloads")
pref = {
    "download.default_directory" : download_dir,
    "download.prompt_for_download": False,
    "download.directory_upgrade": True,
    "safebrowsing.enabled": True
}
chrome_options.add_experimental_option("prefs", pref)
driver = webdriver.Chrome(options=chrome_options)

driver.get("https://www.einvoice.nat.gov.tw/accounts/login")
try:
    login_input = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.ID, "mobile_phone"))
    )
    login_input.clear()
    key = Account
    for char in key:
        login_input.send_keys(char)
        time.sleep(0.1)
    pass_input = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.ID, "password"))
    )
    pass_input.clear()
    key = Password
    for char in key:
        pass_input.send_keys(char)
        time.sleep(0.1)
    captcha_element = driver.find_element(By.CSS_SELECTOR, "img[alt='圖形驗證碼']")
    captcha_element.screenshot("captcha.png")
    ocr = ddddocr.DdddOcr()

    with open("captcha.png", "rb") as file:
        img_bytes = file.read()
        res = ocr.classification(img_bytes)

    capt_input = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.ID, "captcha"))
    )
    capt_input.clear()
    key = res
    for char in key:
        capt_input.send_keys(char)
        time.sleep(0.1)

    login_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.ID, "submitBtn"))
    )
    login_btn.click()

    srch_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.CSS_SELECTOR, "button[title='查詢']"))
    )
    srch_btn.click()

    dropdown_element = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.ID, "SelectSizes"))
    )
    WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.CSS_SELECTOR, "#SelectSizes option[value='100']"))
    )
    select = Select(dropdown_element)
    select.select_by_value("100")

    impl_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.CSS_SELECTOR, "button[title='執行']"))
    )
    driver.execute_script("arguments[0].click();", impl_btn)

    all_checkbox = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.CSS_SELECTOR, "label[for='invoiceDetailAll']"))
    )
    all_checkbox.click()

    time.sleep(3)
    actions = ActionChains(driver)

    download_file = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.CSS_SELECTOR, "button[title='下載CSV檔']"))
    )
    actions.move_to_element(download_file).perform()
    download_file.click()
    time.sleep(5)
    driver.quit()

except Exception as e:
    driver.save_screenshot("error_screenshot.png") # 儲存當下的畫面
    print("發生錯誤，已截圖存檔")
    raise e

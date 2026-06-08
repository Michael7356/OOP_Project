import os
import time
import csv
from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from webdriver_manager.chrome import ChromeDriverManager
import json

# ==========================================
#  使用者設定區：請在這裡輸入你的一卡通卡號
# ==========================================

def load_config():
    base_path = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(base_path, "..", "cmake-build-debug/Storage", "config.json")

    with open(config_path, "r", encoding='utf-8') as f:
        config = json.load(f)
    return config
config = load_config()

CARD_NUMBER = config["IPassCardNumber"]
IDL4 = config["IDLastFourDigits"]

IPASS_URL = "https://www.i-pass.com.tw/Inquire/"


def start_crawler():
    print("====== 一卡通雙表格智慧捕獲器啟動中 ======")

    options = webdriver.ChromeOptions()
    options.add_argument("--window-size=1920,1080")
    #  關鍵修正：禁用瀏覽器的通知彈出視窗（封鎖通知權限請求）
    options.add_argument("--disable-notifications")

    #  選項：關閉自動化測試的提示列（可加可不加，加了畫面更乾淨）
    options.add_experimental_option("excludeSwitches", ["enable-logging"])
    driver = webdriver.Chrome(service=Service(ChromeDriverManager().install()), options=options)

    try:
        driver.get(IPASS_URL)
        time.sleep(3)
        try:
            card_input = driver.find_element(By.ID, "CardId")
            card_input.clear()
            card_input.send_keys("1501102503310516")

            card_input = driver.find_element(By.ID, "IDL4")
            card_input.clear()
            card_input.send_keys(IDL4)

        except Exception:
            print("[提示] 自動填入卡號失敗，請等一下手動輸入。")

        next_input = driver.find_element(By.CSS_SELECTOR, "input[type='submit']")
        next_input.click()
        time.sleep(5)

        impl_input = driver.find_element(By.ID, "paginate_count_of_page")
        impl_input.clear()
        try:
            impl_input.send_keys("99")
        except Exception:
            sj = 0

        print("\n[串流] 開始撈取網頁原始碼並解析...")
        page_html = driver.page_source
        soup = BeautifulSoup(page_html, 'html.parser')

        #  【關鍵修正：篩選過濾雙表格】
        # 找出網頁內所有的 table
        all_tables = soup.find_all('table')
        target_table = None

        for table in all_tables:
            # 檢查這個表格內有沒有包含第二個表格才有的關鍵字 "交易場所"
            if "交易場所" in table.text:
                target_table = table
                break

        if not target_table:
            print("[錯誤] 找不到交易明細表格！請確認網頁下方是否有成功跑出明細。")
            return

        # 找到明細表格後，抓出裡面所有的資料列 (tr)
        rows = target_table.find_all('tr')
        print(f"[成功] 已自動跳過統計圖表，精準鎖定明細表格！共找到 {len(rows)} 行欄位。")
        driver

        csv_filename = "downloads/ipass_records.csv"
        with open(csv_filename, 'w', newline='', encoding='utf-8-sig') as f:
            writer = csv.writer(f)
            writer.writerow(['Date', 'Time', 'Type', 'Location', 'Amount'])

            success_count = 0
            for row in rows:
                cols = row.find_all('td')
                # 根據截圖，明細表格的一行通常有 7 個 td (項次, 交易時間, 類別, 場所, 金額, 餘額, 社福)
                if len(cols) >= 5:
                    # 如果這行的第一個單元格寫的是數字（代表是明細資料列，而非 Header）
                    if not cols[0].text.strip().isdigit():
                        continue

                    raw_datetime = cols[1].text.strip()  # "2026/06/05 14:41:00"
                    record_type = cols[2].text.strip()  # "出站" / "進站" / "購貨"
                    location = cols[3].text.strip()  # "臺北捷運-新埔"
                    raw_amount = cols[4].text.strip()  # "-25" 或 "-0"

                        #  2. 轉換日期格式
                    try:
                        # 處理網頁上可能換行或有多餘空白的時間格式
                        lines = [line.strip() for line in raw_datetime.split('\n') if line.strip()]
                        if len(lines) == 2:
                            date_part, time_part = lines[0], lines[1]
                        else:
                            date_part, time_part = raw_datetime.split(' ')

                        clean_date = date_part.replace('/', '')  # "2026/06/05" -> "20260605"
                    except Exception:
                        clean_date = "00000000"
                        time_part = "00:00:00"

                    raw_amount = raw_amount.replace(',', '')
                    amount_int = int(raw_amount)

                    # 寫入 CSV
                    writer.writerow(["IPass", clean_date, time_part, record_type, location, amount_int])
                    success_count += 1

        print(f"\n 匯出成功！已自動過濾雙表格，清洗出 {success_count} 筆有效消費至 {csv_filename}！")

    finally:
        driver.quit()
        print("======  捕獲器安全下線 ======")


if __name__ == "__main__":
    start_crawler()
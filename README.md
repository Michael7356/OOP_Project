<h1>高度客製化自動記帳及對帳軟體</h1>

<h2>介紹</h2>
<h3>這是一個基於**C++20**標準及**Python自動爬蟲**所打造的智慧記帳及對帳系統，本系統自動可以抓取一卡通、雲端電子發票的資訊，也可以以手動置入檔案的方式讀取銀行消費資訊(目前僅支援中國信託及中華郵政)。</h3>

<h2>核心功能</h2>
<h4>-一卡通及發票資料自動化爬蟲：利用 Python + Selenium 動態模擬登入，提取其資料並解析。</h4>
<h4>-智慧對帳：C++ 核心邏輯透過時間差與金額比對算法，自動將「發票明細（具備精準品項）」與「一卡通扣款（僅具備通路名稱）」等資料進行配對，並讓一卡通紀錄或銀行消費紀錄自動繼承精準的消費類別。</h4>
<h4>-現代 C++ 數據分析：利用 C++20 `<chrono>` 動態獲取系統時間，統計當月消費總計。</h4>

<h2>注意事項</h2>
<h4>
  1.C++編譯器需支援C++20標準或以上<br>
  2.Python版本須為3.8或以上，且須安裝 selenium、 beautifulsoup4和 webdriver-manager庫
</h4>

 <h2>關於Google Script獲取方式</h2>
 
  <h4>**注:有要使用即時獲取消費訊息功能的才需要知道** </h4>
  <h4>1.建立試算表：打開你的雲端硬碟，建立一個全新的 Google 試算表。 </h4>
  <h4>2.打開擴充功能：點擊頂部選單的 「擴充功能 (Extensions)」 並點擊 「Apps Script」。 </h4>
  <h4>3.貼上程式碼：清空裡面預設的 myFunction，將下方的程式碼完整貼進去。</h4>
  <br>
<details>
<summary> 點擊展開查看 Google Apps Script 代碼</summary>

```javascript
function doPost(e) {
  try {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    var data = JSON.parse(e.postData.contents);
    
    // 確保有 message 欄位
    if (!data.message) return ContentService.createTextOutput("No message found");

    sheet.appendRow([new Date(), data.message, 0]);
    
    // 強制更新，確保資料立刻寫入
    SpreadsheetApp.flush(); 
    
    return ContentService.createTextOutput("Success");
  } catch(f) {
    return ContentService.createTextOutput("Error: " + f.toString());
  }
}

function doGet() {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var rows = sheet.getDataRange().getValues();
  var results = [];
  
  // 檢查是否有資料（rows.length > 1 代表除了標題還有資料）
  if (rows.length > 1) {
    for (var i = 0; i < rows.length; i++) {
      // 確保該列有足夠的欄位，且狀態為 0
      if (rows[i].length >= 3 && rows[i][2] === 0) {
        results.push({
          message: rows[i][1]
        });
        
        // 標記為已讀 (1)
        sheet.getRange(i + 1, 3).setValue(1);
      }
    }
  }

  // 重要：標記完後強制存檔，確保下次 doGet 不會重複抓取
  SpreadsheetApp.flush(); 

  // 回傳 JSON
  return ContentService.createTextOutput(JSON.stringify(results))
                       .setMimeType(ContentService.MimeType.JSON);
}
```
</details>

<h4>
  4.點擊右上角的 「部署 (Deploy)」然後點選「新增部署 (New deployment)」。<br><br>
  5.點擊左側的齒輪圖示（選取類型），選擇 「網頁應用程式 (Web app)」。<br><br>
  6.執行身分 (Execute as)：選擇 「我 (Me)」。<br><br>
  7.誰有權限存取 (Who has access)：務必選擇「任何人 (Anyone)」（這樣你的 Python 程式碼才能在不進行複雜 OAuth 認證的情況下，直接透過 POST 打進來）。<br><br>
  8.點擊 部署 (Deploy)。<br>
  9.點擊部署旁的箭頭並選擇「管理部屬作業」
  10.找到「部署作業ID」並複製，此ID即為Google Script
</h4>

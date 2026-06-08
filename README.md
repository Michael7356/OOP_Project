<h1>高度客製化自動記帳及對帳軟體</h1>

<h2>介紹</h2>
<h3>這是一個基於C++20標準及Python自動爬蟲所打造的智慧記帳及對帳系統，本系統可以自動抓取一卡通、雲端電子發票的資訊，也可以以手動置入檔案的方式讀取銀行消費資訊(目前僅支援中國信託及中華郵政)。</h3>

## 核心功能
- 一卡通及發票資料自動化爬蟲：利用 Python + Selenium 動態模擬登入，提取其資料並解析。
- 智慧對帳：C++ 透過時間差與金額比對算法，自動將「發票明細（具備精準品項）」與「一卡通扣款（僅具備通路名稱）」等資料進行配對，並讓一卡通紀錄或銀行消費紀錄自動繼承精準的消費類別。
- 現代 C++ 數據分析：利用 C++20 `chrono` 動態獲取系統時間，統計當月消費總計。

<h2>環境需求</h2>
<h4>
  <h3>C++</h3>
  1.編譯器需支援C++20標準以上<br>
  2.CMake 3.20+<br>
  3.建議開發IDE: JetBrain CLion<br>
  <h3>Python</h3>
  1.Python版本在3.8以上<br>
  2.需安裝Selenium、beautifulsoup4和webdriver-manager 庫
</h4>

<h2>環境建置</h2>
<h4>
  <h3>1.Powershell/Terminal</h3>
  1.打開終端機，並以指令將專案下載至本機<br>

```bash
git clone https://github.com/Michael7356/OOP_Project.git
cd OOP_Project
```
  
  2.建立Python虛擬環境<br>

```bash
cd Python_auto

#如果沒有uv環境
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
#這段可以選擇不裝，但裝了會更好
uv venv .venv
uv pip install selenium beautifulsoup4 webdriver-manager requests
```

  <h3>2.CLion IDE</h3>
  1.以CLion開啟OOP_Project根目錄<br>
  2.綁定Python虛擬環境
  <details>
    <summary>CLion操作說明</summary>
  <h6>
    1.在左側專案樹狀圖中，點開 Python_auto/ 資料夾。<br><br>
    2.點擊 CLion 右下角的 Python Interpreter（或按下 Ctrl + Alt + S 進入 Settings -> 搜尋 Python Interpreter）。<br><br>
    3. 點選 Add Interpreter... -> Virtualenv Environment。<br><br>
     4. 關鍵設定（點選 <b>New</b> 新增環境）：<br>
        - <b>Location</b>：點選右側資料夾圖示，導向到專案中的 <code>Python_auto</code> 目錄，並在路徑尾端<b>手動補上 <code>/.venv</code></b>（CLion 偵測到後會自動在背景為你建立此虛擬環境資料夾）。<br>
        - <b>Base Interpreter</b>：選擇您本機系統安裝的實體 <code>python.exe</code>。<br><br>
     5. 點擊 OK，靜待 CLion 自動初始化環境並安裝基礎套件。
  </h6>
  </details>
</h4>

<h2>使用說明</h2>
<p>
  1.初次使用會顯示初始設定輸入，需輸入電子發票帳號和密碼、一卡通背後卡號及身分證後四碼<br>
  <img width="783" height="145" alt="image" src="https://github.com/user-attachments/assets/a6b2da41-6024-4d04-b56d-bc3c876001a6" />
  2.輸入完之後，會進入主畫面，其中有5個主要功能: <br><br>
  <details>
    <summary>Transaction(交易)</summary>
    按'1'進入交易功能後，會見到如下畫面<br><br>
    <img width="235" height="83" alt="image" src="https://github.com/user-attachments/assets/a42613bd-fed6-4fbb-bf13-f2fdc2eaaa77" /><br>
    <h4>1.Add Transaction(添加交易)</h4>
    選擇'1'進入添加交易，可以輸入交易日期、時間、類別(交易地點)、金額和註記，其中如果除註記外不輸入會有預設值(日期預設為今天，其他則預設為沒有)，填完後會有確認表格，確認正確就會加入交易列表，如果填寫錯誤可以重新填，也可以丟棄資料。<br>
    <img width="784" height="353" alt="image" src="https://github.com/user-attachments/assets/328a77f7-e2da-4c7c-a629-b918e19eba55" /><br>
    <h4>2.Show Transaction(顯示交易)</h4>
    選擇'2'進入顯示交易，會先詢問需要發票記錄還是詳細記錄(即所有紀錄)，選擇發票紀錄會將交易紀錄中發票的項目顯示出來，而選擇詳細記錄則會把所有紀錄都顯示出來。<br>
    在顯示表格時，可以按'n'來抵達下一頁、'l'回到上一頁、's'搜尋類別或註記(品項)、'd'刪除資料、'j'跳到特定頁數、'f'篩選特定分類資料、'q'退出
    <img width="1257" height="137" alt="image" src="https://github.com/user-attachments/assets/b0481240-f0f0-4c43-80b2-560ed796c587" />
  </details>
  <details>
    <summary>Category(類別)</summary>
    按'2'進入類別功能後，會見到如下畫面<br><br>
    <img width="191" height="72" alt="image" src="https://github.com/user-attachments/assets/314d5b48-0b1d-40ec-b0d3-51a0c0948539" />
    <h4>1.Add Category(添加類別)</h4>
    選擇'1'進入添加類別，如果想要將已有的類別連結到另一特定類別，選擇y進入，會見到可以被連結的類別，以其類別之索引輸入進去後，會顯示想要將此類別連結到何類別，填寫完畢後也會有確認表格，正確就會放入類別列表，輸入完後也會能連結其他類別; 如果選擇n則進入模糊連結模式，輸入完類別後，再輸入一個特定類別你想連結至，最後確認即加入類別列表，被連結到的類別會被自動歸類為其連結到的類別。<br>
    (註:如果想要大批量添加類別建議還是到cate.json檔案親自輸入會比較快)
    <img width="630" height="226" alt="image" src="https://github.com/user-attachments/assets/5b2e288e-6361-421d-8eaa-a6e711efc1b3" /><br>
    連結類別Example到example1，之後所有Example類別的都會被連結到example1
    <h4>2.Show Category(顯示類別)</h4>
    選擇'2'進入顯示類別，此功能會顯示已有的類別連結表，也可以在此處按'e'並輸入索引刪除掉連結類別(可以一次刪除多個連結類別[以空白相隔])<br>
    <img width="458" height="69" alt="image" src="https://github.com/user-attachments/assets/4644c437-563b-483e-9424-c9f15d188e78" /><br>
  </details>

  <details>
    <summary>Importing data(引入資料)</summary>
    按'3'進入引入資料功能，此功能可以引進銀行(需自行準備.pdf或.csv檔)、一卡通(自動收取近三個月資料)、電子發票(可自動收取這個月，亦可以.csv檔案輸入資料)
    <h4>Bank and IPass(銀行跟一卡通)</h4>
    按'1'引進銀行或一卡通資料，可以選擇中國信託、中華郵政或是一卡通<br>
    <img width="349" height="181" alt="image" src="https://github.com/user-attachments/assets/85afa335-4d98-4fc4-874a-9111d945d28c" /><br>
    1.中國信託: 需要中國信託網銀取得歷史資料pdf<br>
    2.中華郵政: 由gmail收取對帳單、從網銀取得歷史資料csv檔案皆可引入<br>
    3.一卡通則會自動使用Python爬蟲程式收取近三個月的資料(註:該功能有機會被Recaptcha攔下，如果遇上建議完成Recaptcha後退出瀏覽器再重啟Python程式)<br>
    選擇完後，如為中國信託或中華郵政則需透過選擇檔案視窗選擇正確檔案<br>
    完成後，會有確認表格，裡面為此次擷取到的資料，可以在此選擇刪除資料，退出則自動儲存進交易列表
    <img width="1108" height="303" alt="image" src="https://github.com/user-attachments/assets/8fd8552b-a235-428a-a585-b3060c4e2a0c" /><br>
    <h4>2.Receipt(發票)</h4>
    按'2'引進發票資料，可以按'1'選擇以Python爬蟲程式自動化抓取此月資料，也可以選擇'2'選擇以.csv檔案引入(資料須於財政部雲端電子發票平台獲取.csv檔)<br>
    (註:自動化引進有可能會有過期書籤問題，請退出瀏覽器並重啟Python程式)
    <img width="534" height="159" alt="image" src="https://github.com/user-attachments/assets/7b481787-c8bc-4561-a789-a930572d8e81" /><br>
    完成後，同樣會有確認表格，確認完退出即儲存進交易列表
    <img width="1097" height="311" alt="image" src="https://github.com/user-attachments/assets/753557a8-1030-401a-a1cc-410c6fd1b4db" />
  </details>
  <details>
    <summary>Refresh(重整)</summary>
    如果碰到部分分類未完全分類或排序的情況，使用此功能應該能重新排序並分類分好
  </details>
  <details>
    <summary>Exit with/without store data(退出並保存或不保存)</summary>
    按'5'或'6'退出，如果要將目前的資料存入.csv檔案則選'5'，不想要存入資料則選'6'
  </details>
  
  <h3>注意:</h3>
  此程式非常依賴類別連結，因為不同來源的資料帶有不同的類別標籤，需要以類別連結來將不同來源的類別分類至同一類，如此才能夠配對發票以及消費紀錄
  <img width="1084" height="278" alt="image" src="https://github.com/user-attachments/assets/4a462bc0-67f2-4206-bc29-4827f14dddcb" /><br>
  當類別都分類好時，理想情況可以長這樣，發票旁標著消費來源為何，並且具有日期、部分具有當下的時間，也有消費金額及消費品項(註解)

</p>


<h2>如何由iPhone傳遞資料</h2>
<p>
  註:Android手機我不確定有沒有此功能，這裡會講iPhone捷徑的設定<br>
  註2:此資料僅限來自中華郵政的訊息<br>
  1. 打開 iPhone 內建的 <b>「捷徑 (Shortcuts)」</b> App，點擊下方「自動化」，並點擊右上角「+」新增捷徑。<br><br>
  2. 點擊<b>「訊息」</b>，發送者選為中華郵政發送訊息用的號碼，包含內容輸入每次發送過來都會包含的詞，並選擇及時執行。<br><br>
  3. 點擊<b>「繼續」</b>，在選單中選擇<b>「建立新的捷徑」</b>後，在列表搜尋並選擇 <b>「Get content of URL」</b>。<br><br>
  4. 在<b>「URL」</b>的位置輸入在Google Script中獲取的網頁應用程式網址，點選URL右邊的箭頭，方式選擇<b>「POST」</b>，要求內文為<b>「JSON」</b><br><br>
  5. 點擊<b>「加入新欄位」</b>旁的<b>「+」</b>，選擇<b>「文字」</b>並在<b>「鍵值」</b>輸入<b>「message」</b>，<b>「文字」</b>欄位選擇<b>「捷徑輸入」</b>，最後點擊<b>「捷徑輸入」</b>並選擇<b>「內容」</b>(類型是訊息)<br>
  註:完成後可以按下方的播放鍵，並前往google script存放的google sheet查看是否有資料進入
  <img width="1765" height="93" alt="image" src="https://github.com/user-attachments/assets/d27afb21-a780-41dc-a68c-9d971310c8ae" />
  正常會長這樣(當有訊息傳入)

 <h2>關於Google Script獲取方式</h2>
 
  <p>1.建立試算表：打開你的雲端硬碟，建立一個全新的 Google 試算表。 <br>
  <br>2.打開擴充功能：點擊頂部選單的 「擴充功能 (Extensions)」 並點擊 「Apps Script」。 <br>
  <br>3.貼上程式碼：清空裡面預設的 myFunction，將下方的程式碼完整貼進去。</p>
</p>
  
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

<p>
  4.點擊右上角的 「部署 (Deploy)」然後點選「新增部署 (New deployment)」。<br><br>
  5.點擊左側的齒輪圖示（選取類型），選擇 「網頁應用程式 (Web app)」。<br><br>
  6.執行身分 (Execute as)：選擇 「我 (Me)」。<br><br>
  7.誰有權限存取 (Who has access)：務必選擇「任何人 (Anyone)」（這樣你的 Python 程式碼才能在不進行複雜 OAuth 認證的情況下，直接透過 POST 打進來）。<br><br>
  8.點擊 部署 (Deploy)。<br><br>
  9.點擊部署旁的箭頭並選擇「管理部屬作業」<br><br>
  10.找到「部署作業ID」並複製，此ID即為Google Script
</p>
<br>

<h2>
  授權與宣告
</h2>
<h5>
  本專案內建之 nlohmann/json 採用 MIT License。<br><br>
  本專案內建之 yhirose/cpp-httplib 採用 MIT License。<br><br>
  本專案核心代碼由開發者自主設計，僅供學術交流與個人記帳使用。<br><br>
</h5>

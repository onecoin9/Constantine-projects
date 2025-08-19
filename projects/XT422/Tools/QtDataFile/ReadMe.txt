Qt安裝到C:\Qt後
檔案總管裡 "This PC" 滑鼠右鍵選Properties
選 "Advanced system settings" 裡的 "Advanced" Tab
下方 <Environment Variables> 按鈕

開啟環境變數Dialog, 到下方的System Variables
<New...> 創一個 QTDIR 內容是 C:\Qt\5.15.2\msvc2019
點選 Path 編輯,在一堆路徑裡加入兩個
%QTDIR%\bin
%QTDIR%\lib

這樣給Visual Studio的Qt Extension或Qt Creator用的環境變數就設定好了
剛編好系統環境變數, 需要重開機讓他生效

這樣我們不用去給每個Project的.vcxproj.user增加各種QTDIR在 platform輸出組合
因為系統變數中指定好的Compiler不論x86/x64對IDE本就可以應付x86/x64各種輸出

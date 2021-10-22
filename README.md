OS跟網路程式筆記
---

- [simple shell](shell)
  - [demo pic](img/shell_1.png)
  - 以C實作的簡易shell，支援一般指令、背景執行（&）、multiple pipeline。
  - 以double fork防止zombie process（主要用於背景執行）。
  - 以recursive實現multiple pipeline的fd處理。
  - 不支援`cd`、redirect dataflow等操作。
- [chatroom](chatroom)
  - [demo pic](img/chatroom_1.png)
  - 用pthread跟socket實現的多人線上聊天室，但礙於那時對socket了解有限，部分架構不夠優美。
- [matrix production](matrix_production)
  - [demo pic](img/matrix_production_1.png)
  - 分別用multithread跟multiprocess實現平行化的矩陣乘法，目的在於直觀感受平行化的加速。
  - 由於使用機器cpu有限，超過8個thread／process後沒有顯著改善。
  - thread的表現略優於process，猜測是process switch成本更高的原因。
- [BBS](BBS)
  > 感謝jason提醒多線程中，`strtok`有共用記憶體的問題，改用`strtok_r`實現
  - [demo pic](img/BBS_1.png)
  - 以OOP的概念實作BBS。
  - 為了在多人交互時，不同時修改同一個記憶體導致資料損毀，使用排他鎖及讀寫鎖維護。
  - server端操作： 
    - 編譯指令：`g++ *.cpp`
    - `./a.out <IP> <port> <max user>`
    - IP僅能使用本機上網卡的IP，或以0.0.0.0代表本地IP
    - Port須大於1024（1024前為通用port）
    - max user不為負數
  - client端操作：
    - `nc <IP> <port>`：連線server
    - `register <username> <password>`：註冊用戶
    - `login <username> <password>`：登入用戶
    - `whoami`：查詢當前帳號名稱
    - `list-user`：列出所有用戶
    - `send <username> <message>`：給指定用戶訊息
    - `list-msg`：分別列出從各用戶收到的信息數量
    - `receive <username>`：列出從用戶收到的所有訊息
    - `logout`：登出
    - `exit`：中斷連線



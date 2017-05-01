# NetworkedEditor
A networked text editor

# Key Combinations:
  - `Ctrl+W`: change windows
  - `Ctrl+Q`: exit the editor (without saving)
  - `Ctrl+S`: force a sync with the server (should  not be required as this happens periodically anyways)
  - `:w <filename>` in the command window: save the file on the server
  - `:wl <filename>` in the command window: save the file locally
  - `:e <filename>` in the command window: load a file on the server
  - `:cc` in the command window: get the number of clients currently connected

# Command Line Options:
  - `-a`:  address to connect to (string)
  - `-sp`: port to connect to (string)
  - `-p` : port to start server on (string)
  - `--server`: start the server, even if the -a flag exists

If -a is set, a server is not launched, unless --server is also specified

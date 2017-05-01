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

If -a is set, a server is not launched, unless --server is also specified.

### Example Usage
`./editor` normal invocation
`./editor -a localhost` connect to an already running server on this machine
`./editor -a 192.168.1.100` connect to a server running on the IP 192.168.1.100 using the default port

# More Details
   - Compiling this editor requires g++ 4.9 or above due to the use of C++11 regex. 
   - Compiling this editor requires ncurses to be installed.
   - This uses ncurses default coloring system, so make sure your terminal supports colors to see the syntax highlighting
   - The default port is 29629, so make sure this isn't blocked

# LogAnalyzer
A windows-based C application that can read a file of 'log entries' and extract information about it.
Every entry must have this syntax: <user>@<date>-<time>-<operation>-<type of entry>-<outcome of the entry>-<execution time>
  - user is a string containing the user that executed the operation
  - date and time are in the form of dd/mm/yy and hh:mm:ss, they are the date and time the log would have been written
  - operation is a string that explains what operation has been done (eg. write, read, execute, or something more precise such as 'server socket opened at port <number>')
  - The type of entry should be an indicator to what the entry is for, it can be either Information, Warning or Error
  - Outcome is either Success or Failure, and explains how the operation ended
  - Execution time is a double value containing the duration the operation lasted, so either the time until completition or until crash

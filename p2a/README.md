#p2a
Teryl Schmidt | 9072604920 | CS537 | 2019  

User function loops printing 'wish> ' and getting user input  
Once user enters line we parse if and excecute that line with the execfn  
This function checks if the user input one of the built in functions: 'exit' 'cd' 'path' and executes those from the "/bin" directory  
The function also checks if 'history' is typed or '|' or '>'  
redirection function is for '>' which redirects output to file specified  
parallel run function helps the redirection function  
history is an array with a add and print function and a parse_ulong function to get the number the user entered for how far back they want to go in history  
mypipe function helps with the piping command  


#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print ("HTTP/1.1 200 OK")
print ("Content-type: text/html")
print ("")
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print ("<a href=\"/\">Go to home page</a>")


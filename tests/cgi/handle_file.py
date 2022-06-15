#!/usr/bin/env python3
import cgi, os
import cgitb; cgitb.enable()
# cgi.test()
form = cgi.FieldStorage(environ="post")
# Get filename here.
print("form in handle_file:", form)
exit();
fileitem = form['filename']
print(form['filename'])
print(form['filename'].value)
filename = form['filename'].value
# Test if the file was uploaded
if filename:
   # strip leading path from file name to avoid
   # directory traversal attacks
   fn = os.path.basename(filename.replace("\\", "/" ))
   open('/tmp/' + fn, 'wb').write(fileitem.file.read())
   message = 'The file "' + fn + '" was uploaded successfully'
else:
   message = 'No file was uploaded'
print ("""\
Content-Type: text/html\n
<html>
<body>
   <p>%s</p>
</body>
</html>
""" % (message,))

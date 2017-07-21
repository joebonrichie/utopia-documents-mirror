# -*- coding: utf-8 -*-
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
  <head>
  <title>PDF Text Extractor</title>
  </head>
  <body>
    <h1>PDF Text Extractor</h1>

    ${h.form(h.url_for(action='upload'), multipart=True)}
    Upload file:      ${h.file('pdfdoc')}
                      ${h.submit('Submit', 'Submit')}
    ${h.end_form()}
  </body>
</html>

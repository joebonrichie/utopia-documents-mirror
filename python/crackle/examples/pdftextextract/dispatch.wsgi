import os, sys
sys.path.append('/opt/pdftextextract/')

from paste.deploy import loadapp

application = loadapp('config:/opt/pdftextextract/production.ini')

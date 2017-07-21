%pythonappend Annotation::Annotation() %{
    self.beginRegion()
%}


%extend Image {
    %pythoncode %{
        def toPILImage(self):
            from PIL import Image
            import StringIO

            result=None

            if self.type()==BitmapImage:
                result= Image.fromstring('1', self.size(), self.data())

            elif self.type()==RGBImage:
                result= Image.fromstring('RGB', self.size(), self.data())

            elif self.type()==JPEGImage:
                result= Image.open(StringIO.StringIO(self.data()))

            return result
    %}
}


%extend Cursor {
  %pythoncode %{

    def image(self):
        return self._image().toPILImage()

    class CursorIterator:

        def __init__(self, cursor, advance_meth, available_meth, limit):
            self._cur= cursor.copy()
            self._advance_meth= advance_meth
            self._available_meth=available_meth
            self._limit= limit

        def __iter__(self):
            return self

        def next(self):
            result=None
            if not self._available_meth(self._cur):
                raise StopIteration
            result=self._cur.copy()
            self._advance_meth(self._cur, self._limit)
            return result

    def pages(self, limit=UntilEndOfDocument):
        return Cursor.CursorIterator(self, Cursor.advancePage, Cursor.pageValid, limit)

    def regions(self, limit=UntilEndOfPage):
        return Cursor.CursorIterator(self, Cursor.advanceRegion, Cursor.regionValid, limit)

    def blocks(self, limit=UntilEndOfRegion):
        return Cursor.CursorIterator(self, Cursor.advanceBlock, Cursor.blockValid, limit)

    def lines(self, limit=UntilEndOfBlock):
        return Cursor.CursorIterator(self, Cursor.advanceLine, Cursor.lineValid, limit)

    def words(self, limit=UntilEndOfLine):
        return Cursor.CursorIterator(self, Cursor.advanceWord, Cursor.wordValid, limit)

    def characters(self, limit=UntilEndOfWord):
        return Cursor.CursorIterator(self, Cursor.advanceCharacter, Cursor.characterValid, limit)

    def images(self, limit=UntilEndOfPage):
        return Cursor.CursorIterator(self, Cursor.advanceImage, Cursor.imageValid, limit)
  %}
}


%extend Document {
  %pythoncode %{

    def pages(self, limit=UntilEndOfDocument):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advancePage, Cursor.pageValid, limit)

    def regions(self, limit=UntilEndOfPage):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceRegion, Cursor.regionValid, limit)

    def blocks(self, limit=UntilEndOfRegion):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceBlock, Cursor.blockValid, limit)

    def lines(self, limit=UntilEndOfBlock):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceLine, Cursor.lineValid, limit)

    def words(self, limit=UntilEndOfLine):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceWord, Cursor.wordValid, limit)

    def characters(self, limit=UntilEndOfWord):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceCharacter, Cursor.characterValid, limit)

    def images(self, limit=UntilEndOfDocument):
        return Cursor.CursorIterator(self.newCursor(), Cursor.advanceImage, Cursor.imageValid, limit)

    def render(self, page, resolution):
        return self._render(page, resolution).toPILImage()

    def renderArea(self, area, resolution):
        return self._renderArea(area, resolution).toPILImage()

    def search(self, regex, options=DefaultSearchOptions, start=None):
        if start is not None:
            return self._searchFrom(start, regex, options)
        else:
            return self._search(regex, options)

    def findInContext(self, before, label, after, fuzzy = True):
        import re
        import spineapi

        def fuzz(input, strict = False):
            if fuzzy:
                def repl(match):
                    length = len(match.group(0))
                    if strict:
                        return '.'*length
                    else:
                        return '.{{{0},{1}}}'.format(max(0, length-4), length+3)
                return r'\.'.join((re.sub(r'\\,', r',\s?',
                                   re.sub(r'[^\w\d.\\,]+', repl,
                                   re.sub(r',', r'\,',
                                   re.sub(r'(^[a-zA-Z0-9#]*;|&[^\s;]+;|&[a-zA-Z0-9#]*$)', r'.', token))))
                                   for token in input.split('.')))
            else:
                return input

        #print (before, label, after)
        before, label, after = (fuzz(before), fuzz(label, strict = True), fuzz(after))
        #print (before, label, after)
        regex = "%s.?(%s).?%s" % (before, label, after)

        # Now ignore whole matches, and get only the sub-string matches
        return self.search(regex, spineapi.RegExp + spineapi.IgnoreCase)[1::2]

    def annotations(self, scratch = None):
        if scratch is None:
            return self._annotations()
        else:
            return self._scratchAnnotations(scratch)

    def annotationsIf(self, filters, scratch = None):
        all = self.annotations(scratch)
        matching = set([])
        for annotation in all:
            matches = True
            for key in filters:
                if key not in annotation:
                    matches = False
                    break
            if matches:
                try:
                    for key, value in filters.iteritems():
                        if value is not None and annotation[key] != value:
                            matches = False
                            break
                except:
                    pass
            if matches:
                matching.add(annotation)
        return matching

    def newAccList(self, accType, rank = None):
        import spineapi
        link = spineapi.Annotation()
        link['session:volatile'] = '1'
        link['concept'] = 'AccumulatorListLink'
        link['scratch'] = self.newScratchId()
        link['type'] = accType
        if rank is not None:
            link['rank'] = rank
        self.addAnnotation(link, '__master_accumulator_list__')
        return link

    def getAccLists(self, accType):
        filters = {'concept': 'AccumulatorListLink', 'type': accType}
        matches = list(self.annotationsIf(filters, '__master_accumulator_list__'))
        return sorted(matches, key=lambda a: int(a.get('rank', 0)), reverse=True)

    def addAnnotation(self, annotation, scratch = None):
        if scratch is None:
            return self._addAnnotation(annotation)
        else:
            return self._addScratchAnnotation(annotation, scratch)

    def addAnnotations(self, annotations, scratch = None):
        if scratch is None:
            return self._addAnnotations(list(annotations))
        else:
            return self._addScratchAnnotations(list(annotations), scratch)

    def removeAnnotation(self, annotation, scratch = None):
        if scratch is None:
            return self._removeAnnotation(annotation)
        else:
            return self._removeScratchAnnotation(annotation, scratch)

  %}
}

%extend Annotation {
  %pythoncode %{

    def __len__(self):
        return len(self.properties())

    def __getitem__(self, key):
        if key in self:
            return self.getFirstProperty(key)
        else:
            raise KeyError(key)

    def __setitem__(self, key, value):
        import collections
        if isinstance(value, basestring):
            self.setProperty(key, value)
        elif isinstance(value, collections.Iterable):
            try:
                del self[key]
            except KeyError:
                # Ignore
                pass
            for v in value:
                if not isinstance(v, basestring):
                    v = unicode(v)
                self.insertProperty(key, v)
        else:
            self.insertProperty(key, unicode(value))

    def __delitem__(self, key):
        if key in self:
            return self.removePropertyAll(key)
        else:
            raise KeyError(key)

    def __iter__(self):
        return iter(self.properties())

    def __contains__(self, key):
        return self.hasProperty(key)

    def get(self, key, default = None):
        if key in self:
            return self[key]
        else:
            return default

    def iteritems(self):
        return self.properties().iteritems()

    def addAreas(self, areas):
        for area in areas:
            self.addArea(area)

    def addExtents(self, extents):
        for extent in extents:
            self.addExtent(extent)

    def beginRegion(self):
        self.regions=list()
        self.last_font=None
        self.rich_content=None

    def endRegion(self):
        if len(self.regions) > 0:
            regiontxt=''
            for r in self.regions:
                (p, _, (x1,y1), (x2,y2))=r
                regiontxt+="%d@%f:%f:%f:%f;" % (p,x1,y1,x2,y2)
            self.__setitem__("spine:annotation:region:pageboxes", regiontxt)

        if self.rich_content:
            self.__setitem__("spine:annotation:text:richcontent", self.rich_content)
            self.__setitem__("spine:annotation:text:content", self.content())
            self.__setitem__("spine:annotation:type:text", "1")

        return self.rich_content

    def appendPage(self, cursor):
        self.regions.append(cursor.pageArea())
        for w in cursor.words(UntilEndOfPage):
            self._updateContentByWord(w)

    def appendRegion(self, cursor):
        self.regions.append(cursor.regionArea())
        for w in cursor.words(UntilEndOfRegion):
            self._updateContentByWord(w)

    def appendBlock(self, cursor):
        self.regions.append(cursor.blockArea())
        for w in cursor.words(UntilEndOfBlock):
            self._updateContentByWord(w)

    def appendLine(self, cursor):
        self.regions.append(cursor.lineArea())
        for w in cursor.words(UntilEndOfLine):
            self._updateContentByWord(w)

    def appendWord(self, cursor):
        self.regions.append(cursor.lineArea())
        self._updateContentByWord(cursor)

    def appendTextExtent(self, extent):
        self.regions += extent.boundingBoxes()
        # FIXME how to add text content?

    def content(self):
        strippedtxt=self.rich_content
        if strippedtxt:
            strippedtxt.replace('_',' ')
            strippedtxt.replace("\x2017",'_')

        return strippedtxt

    def _updateContentByWord(self, cursor):
        if cursor.wordValid():
            text=cursor.wordText().strip()

            # replace existing underscores with double underscores
            # we will of course fail to preserve existing double underscores
            text.replace('_', "\x2017")

            # add a space or font change character if neccessary
            if self.rich_content:

                # deal with hyphenated lines
                # (hyphenated words should already include the hyphen)
                need_space=True

                # shame python does not understand unicode classes
                if self.rich_content[-1] in [u"-", u"\x058A", u"\x2010", u"\x2013", u"\x2014", u"\xFE63", u"\xFF0D"]:
                    self.rich_content= self.rich_content[0:-1]
                    need_space=False

                # Mark font changes
                if cursor.wordFontName() != self.last_font:
                    self.rich_content+="_"
                    self.last_font=cursor.wordFontName()
                elif need_space:
                    self.rich_content+=" "

            else:
                self.rich_content=''
                self.last_font=cursor.wordFontName()

            # add text
            self.rich_content+= text

  %}
}

%extend TextExtent {
  %pythoncode %{

    def __unicode__(self):
        return self.text()

    def search(self, regex, options=DefaultSearchOptions, start=None):
        if start is not None:
            return self._searchFrom(start, regex, options)
        else:
            return self._search(regex, options)

  %}
}

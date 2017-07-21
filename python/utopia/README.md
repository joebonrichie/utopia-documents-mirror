# Utopia framework

This package provides the basic framework for the Utopia suite. It provides mechanisms for declaring and loading plugins and working with citations. Whereas it can be useful on its own, it is expected to be used alongside other packages that provide tools and plugins for doing useful analysis. This package does not yet work on Python 3+.

## Getting Started

In most cases, you will want the following four packages installed.

  1. **pyutopia** (this package) provides the base mechanisms for defining and loading plugins, adding tools, and manipulating citations.
  1. [**pyutopia-tools**](https://github.com/lostislandlabs/python-utopia-tools) provides utilities to various common resources such as PubMed, Crossref, ArXiv, etc. It also provides access utilities for working with the various Utopia servers. These tools can be used to create bespoke applications.
  1. [**pyutopia-plugins-common**](https://github.com/lostislandlabs/python-utopia-plugins-common) holds all the standard plugins used by the resolution pipeline.
  1. [**kend**](https://github.com/lostislandlabs/kend) provides utilities for accessing a kend server, and for parsing and serialising its data formats.

### Prerequisites

One of the dependencies of the Utopia packages is the [LXML](http://lxml.de/) package, which itself needs to be partly compiled from source that depends on the libxml2 and libxslt2 development libraries. Ensure these are present before continuing.

### Installing

You can either clone these repositories and build them yourself, or you can install the latest stable versions of the packages directly from PyPi:

```
$ pip install pyutopia-plugins-common # will pull in everything needed
```

## Usage

Utopia can be thought of as being in three separate parts: the _framework_, the _tools_, and the _plugins_. The framework is the low-level environment into which the tools and plugins fit, allowing them to extend the core functionality of Utopia via namespace packages and plugin resolution.

### Using Utopia's tools

The [**pyutopia-tools**](https://github.com/lostislandlabs/python-utopia-tools) package provides a set of Python modules for interacting with common services available in the bioinformatics and life sciences spaces. They can be found within the `utopia.tools` package.

#### Using `utopia.tools.arxiv`

The ArXiv tool allows you, given an ArXiv ID, to fetch and parse the article's citation information.

```python
>>> import utopia.tools.arxiv as arxiv
>>> id = '1707.06154'
>>> arxiv.url(id)
'http://arxiv.org/abs/1707.06154'
>>> xml = arxiv.fetchXML(id) # Returns the ATOM item XML for that ID
>>> citation = arxiv.parseXML(xml) # Returns a citation dictionary
>>> citation = arxiv.resolve(id) # Wraps up the previous three steps
```

#### Using `utopia.tools.crossref`

The Crossref tool allows you, given a DOI, to fetch and parse an article's citation information.

```python
>>> import utopia.tools.crossref as crossref
>>> doi = '10.1371/journal.pcbi.1000976'
>>> crossref.url(doi)
'http://dx.doi.org/10.1371/journal.pcbi.1000976'
>>> xml = crossref.fetchXML(doi) # Returns unixref XML for the DOI
>>> citation = crossref.parseXML(xml) # Returns a citation dictionary
>>> citation = crossref.resolve(doi) # Wraps up the previous two steps
```

The tool also allows for a couple of different kinds of searches.

```python
>>> title = 'The Mycobacterium tuberculosis Drugome and Its Polypharmacological Implications'
>>> citations = crossref.search(title) # Returns summary citation for all matches
>>> unstructured = 'Kinnings SL, Xie L, Fung KH, Jackson RM, Xie L, Bourne PE (2010) The Mycobacterium tuberculosis Drugome and Its Polypharmacological Implications. PLoS Comput Biol 6(11): e1000976'
>>> citation = crossref.freeform_search(unstructured) # Matches a full citation to a Crossref article
```

#### Using `utopia.tools.eutils`

The EUtils tool provides utilities for communicating with an EUtils service. There is a factory function for creating general EUtils service stubs.

```python
>>> import utopia.tools.eutils as eutils
>>> my_new_function = eutils.eutils('name', usehistory='y', retmax=100, retmode='xml')
```

But most people will likely use the standard stubs that are already generated for you: `eutils.esearch(...)`, `eutils.espell(...)`, `eutils.efetch(...)`, `eutils.egquery(...)`. These stubs form the basis for both the `utopia.tools.pmc` and `utopia.tools.pubmed` tools.

#### Using `utopia.tools.nlm`

This tool provides a NLM (National Library of Medicine) XML parsing utility.

```python
>>> import utopia.tools.nlm as nlm
>>> xml = '...' # Get some NLM XML data from somewhere
>>> citation = nlm.parse(xml) # Returns the citation information
```

#### Using `utopia.tools.pmc`

The PMC tool provides utilities for communicating with PubMed Central. It makes use of the `utopia.tools.eutils` tool for communicating with the server.

```python
>>> import utopia.tools.pmc as pmc
>>> id = 'PMC2973814'
>>> xml = pmc.fetchXML(id) # Returns the PubMed XML record for an article
>>> pmc.identify('21079673', 'pmid') # Given a PubMed ID, identify its PMC ID
'PMC2973814'
>>> pmc.identify('10.1371/journal.pcbi.1000976', 'doi') # Given a DOI, identify its PMC ID
'PMC2973814'
```

#### Using `utopia.tools.pubmed`

The PubMed tool provides utilities for communicating with PubMed. It makes use of the `utopia.tools.eutils` tool for communicating with the server.

```python
>>> import utopia.tools.pubmed as pubmed
>>> id = '21079673'
>>> xml = pubmed.fetchXML(id) # Returns the PubMed XML record for an article
>>> pubmed.identify('PMC2973814', 'pmcid') # Given a PMC ID, identify its PubMed ID
'21079673'
>>> pubmed.identify('10.1371/journal.pcbi.1000976', 'doi') # Given a DOI, identify its PubMed ID
'21079673'
>>> citation = pubmed.resolve(id) # Returns the citation information for the article
```

The PubMed tool also allows searches to be performed.

```python
>>> title = 'The Mycobacterium tuberculosis Drugome and Its Polypharmacological Implications'
>>> citation = pubmed.search(title) # If a single matching entry is found, returns its citation information
```

## License

This project is licensed under the GPLv3 License - see the [LICENSE.md](LICENSE.md) file for details

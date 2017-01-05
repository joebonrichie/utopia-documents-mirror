Summary:	PDF reader optimised for scientific literature
Name:		utopia-documents
Version:	2.4.2
Release:	%mkrel 3
License:	GPLv3
Group:		Office/Utilities
URL:		http://utopiadocs.com/
Source0:	http://utopiadocs.com/files/linux/%{name}_%{version}.orig.tar.bz2
Patch0:		%{name}_config-build.patch
Patch1:		%{name}_fix-desktop-file.patch
BuildRequires:	zip
BuildRequires:  cmake
BuildRequires:	qt4-devel
BuildRequires:  qtwebkit-devel
BuildRequires:	qjson-devel
BuildRequires:	pcrecpp-devel
BuildRequires:	libx11-devel
BuildRequires:	openssl-devel
BuildRequires:	glew-devel
BuildRequires:	libQGLViewer-devel
BuildRequires:	boost-devel
BuildRequires:	zlib-devel
BuildRequires:	freetype
BuildRequires:	expat-devel
BuildRequires:	libpython-devel
BuildRequires:	swig

%description
Utopia Documents is a free PDF reader that connects the static content of
scientific articles to the dynamic world of online content.

It makes it easy to explore an article's content and claims, and investigate
other recent articles that discuss the same or similar topics, get access to
an article's metadata, generate a formatted citation for use in your own work,
follow bibliographic links to cited articles, get a document's related data, 
and see what other researchers have been saying about the article you're
reading and its subject in social networks.

%prep
%setup -q
%patch0
%patch1

%build
./configure
%make -C build

%install
%makeinstall_std -C build

%files
%{_bindir}/%{name}
%{_prefix}/lib/%{name}
%{_iconsdir}/hicolor/*/apps/*.png
%{_iconsdir}/gnome/*/apps/*.png
%{_datadir}/applications/*.desktop
%{_datadir}/%{name}


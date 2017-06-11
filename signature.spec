Name: signature
Version: 0.14
Release: 1
Summary: a dynamic signature generator for e-mail and news
License: GPL
Group: Amusements/Games
URL: http://www.caliban.org/linux_signature.html
Source: http://www.caliban.org/files/signature/%{name}-%{version}.tar.gz
BuildRoot: /tmp/%{name}-%{version}
Packager: Ian Macdonald <ian@caliban.org>
Requires: fortune-mod

%changelog
* Mon Jul 17 2000  Ian Macdonald <ian@caliban.org> - 0.0
- update to 0.14
- fixed bug that caused '<' placeholder to remain behind when there were
  backspaces or underscores in the text

* Sun Jul  2 2000  Ian Macdonald <ian@caliban.org> - 0.0
- update to 0.13
- white space at end of lines now trimmed
- ^H (backspace) and _ now ignored in fortune text
- added a couple of debugging messages
- fixed bug that caused FIFO and quote file not to be found when using
  -f and -q

* Sun Apr 16 2000  Ian Macdonald <ian@caliban.org> - 0.0
- update to 0.12
- SIGPIPE is now ignored (stops Pine from killing signature)

* Sun Feb 13 2000  Ian Macdonald <ian@caliban.org> - 0.0
- update to 0.11
- external programs can now be called with arguments
- quote files can now use either ~ or % as a separator
-
- EARLIER COMMITS REMOVED

%description
Ever wanted to have witty and pithy sigs attached to your e-mail and
news postings, but got tired having the same old one attached every
time?

That's where signature comes in. This program will allow you to sign
your messages with a different sig every time.

%prep
%setup

%build
%define debug_package %{nil}
        rm -rf $RPM_BUILD_ROOT
        CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" ./configure --enable-offensive
        make

%install
        make prefix=$RPM_BUILD_ROOT/usr mandir=$RPM_BUILD_ROOT/%{_mandir} install

%clean
        rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README TODO
%attr(555,root,root)/usr/bin/signature
%attr(444,root,root)%{_mandir}/man1/signature.1*

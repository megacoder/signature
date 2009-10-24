Name: signature
Version: 0.14
Release: 1
Summary: a dynamic signature generator for e-mail and news
Copyright: GPL
Group: Amusements/Games
URL: http://www.caliban.org/linux_signature.html
Source: http://www.caliban.org/files/signature/%{name}-%{version}.tar.gz
BuildRoot: /tmp/%{name}-%{version}
Packager: Ian Macdonald <ian@caliban.org>
Requires: fortune-mod

%changelog
* Mon Jul 17 2000  Ian Macdonald <ian@caliban.org>
- update to 0.14
- fixed bug that caused '<' placeholder to remain behind when there were
  backspaces or underscores in the text

* Sun Jul  2 2000  Ian Macdonald <ian@caliban.org>
- update to 0.13
- white space at end of lines now trimmed
- ^H (backspace) and _ now ignored in fortune text
- added a couple of debugging messages
- fixed bug that caused FIFO and quote file not to be found when using
  -f and -q

* Sun Apr 16 2000  Ian Macdonald <ian@caliban.org>
- update to 0.12
- SIGPIPE is now ignored (stops Pine from killing signature)

* Sun Feb 13 2000  Ian Macdonald <ian@caliban.org>
- update to 0.11
- external programs can now be called with arguments
- quote files can now use either ~ or % as a separator

* Tue Dec 30 1999  Ian Macdonald <ian@caliban.org>
- update to 0.10
- no-daemon mode is now selected with -n
- -d now provides debugging output on stderr
- support for quote files with -q

* Mon Dec 27 1999  Ian Macdonald <ian@caliban.org>
- update to 0.04
- daemon mode made more daemon like
- build environment uses GNU automake
- support for offensive fortunes now a compile time option

* Mon Dec 20 1999  Ian Macdonald <ian@caliban.org>
- update to 0.03
- now forks into background on start-up. The -d switch has been added
  to prevent this
- when supplying an alternative program to fortune with the -p switch,
  the full path no longer has to be provided
- default sig format string changed for kernel and architecture strings
  of different lengths

* Mon Nov  8 1999  Ian Macdonald <ian@caliban.org>
- update to 0.02
- fix for zombie process bug
- usage message tidied up

* Mon Nov  1 1999  Ian Macdonald <ian@caliban.org>
- first RPM release (0.01)

%description
Ever wanted to have witty and pithy sigs attached to your e-mail and
news postings, but got tired having the same old one attached every
time?

That's where signature comes in. This program will allow you to sign
your messages with a different sig every time.

%prep
%setup

%build
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

Name: gecode
Version: 0.9
Release: 1
Copyright: X11
Summary: The Gecode Constraint Library
Group: Development/Languages
Source0: gecode-%{version}.tar.gz
Vendor: The Gecode Project
URL: http://www.gecode.org
Packager: Guido Tack <tack@ps.uni-sb.de>
Requires: /bin/bash
Requires: /bin/sh
Requires: libc.so.6
Requires: libc.so.6(GLIBC_2.0)
Requires: libc.so.6(GLIBC_2.1)
Requires: libc.so.6(GLIBC_2.1.3)
Requires: libc.so.6(GLIBC_2.2)
Requires: libc.so.6(GLIBC_2.3)
Requires: libdl.so.2
Requires: libdl.so.2(GLIBC_2.0)
Requires: libdl.so.2(GLIBC_2.1)
Requires: libgcc_s.so.1
Requires: libgcc_s.so.1(GCC_3.0)
Requires: libm.so.6
Requires: libm.so.6(GLIBC_2.0)
Requires: libstdc++.so.5
Requires: libstdc++.so.5(GLIBCPP_3.2)

BuildArchitectures: i386
BuildRoot: %{_tmppath}/%{name}-%{version}
AutoReqProv: no

%description
More information can be found on the Gecode web site at http://www.gecode.org.

%prep
rm -rf %{buildroot}
%setup -q -D -a 0 -c

%build
cd gecode-%{version}
%configure

%install
cd gecode-%{version}
%makeinstall

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%{_includedir}/gecode/*
%{_libdir}/gecode/*

%changelog
* Thu Feb 17 2005 Guido Tack <tack@ps.uni-sb.de>
- Beta release

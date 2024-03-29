#
# spec file for package udjat-module-network
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define product_name %(pkg-config --variable=product_name libudjat)
%define module_path %(pkg-config --variable=module_path libudjat)


Summary:		Network module for %{product_name} 
Name:			udjat-module-network
Version:		1.0
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat-module-network

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	pkgconfig(libudjat)
BuildRequires:	pkgconfig(pugixml)

%description
Network module for udjat.

Add factory for %{product_name} network validation and check agents.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%package -n libudjatnetw%{_libvrs}
Summary:	UDJat network library

%description -n libudjatnetw%{_libvrs}
Network abstraction library for %{product_name}

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n udjat-network-devel
Summary:	Development files for %{name}
Requires:	pkgconfig(libudjat)
Requires:	libudjatnetw%{_libvrs} = %{version}

%description -n udjat-network-devel

Development files for Udjat's network abstraction library.

%lang_package -n libudjatnetw%{_libvrs}

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

NOCONFIGURE=1 \
	./autogen.sh

%configure

%build
make all

%install
%makeinstall
%find_lang libudjatnetw-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%files
%defattr(-,root,root)
%{module_path}/*.so

%files -n libudjatnetw%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libudjatnetw.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n libudjatnetw%{_libvrs}-lang -f langfiles

%files -n udjat-network-devel
%defattr(-,root,root)
%{_includedir}/udjat/tools/net/*.h
%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%pre -n libudjatnetw%{_libvrs} -p /sbin/ldconfig

%post -n libudjatnetw%{_libvrs} -p /sbin/ldconfig

%postun -n libudjatnetw%{_libvrs} -p /sbin/ldconfig

%changelog


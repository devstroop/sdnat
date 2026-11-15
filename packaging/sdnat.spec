Name:           sdnat
Version:        %{sdnat_version}
Release:        %{sdnat_release}
Summary:        Portable virtual router
License:        Apache-2.0
URL:            https://github.com/devstroop/sdnat
BuildArch:      %{sdnat_arch}
AutoReqProv:    no
%global __os_install_post %{nil}

%description
SDNAT provides L2/L3 forwarding, bridging, NAT, DHCP, DNS and a
stateful firewall for Linux. This package contains sdnat, sdnatd,
the systemd unit, the C ABI library and the user documentation.

%prep
%build
%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
cp -a %{sdnat_stage}/. %{buildroot}/

%post
if [ -d /run/systemd/system ]; then
    systemctl daemon-reload >/dev/null 2>&1 || :
fi

%preun
if [ "$1" -eq 0 ] && [ -d /run/systemd/system ]; then
    systemctl stop sdnatd.service >/dev/null 2>&1 || :
fi

%postun
if [ -d /run/systemd/system ]; then
    systemctl daemon-reload >/dev/null 2>&1 || :
fi

%files
%defattr(-,root,root,-)
%dir /etc/sdnat
%config(noreplace) /etc/sdnat/sdnat.json.example
/lib/systemd/system/sdnatd.service
/usr/bin/sdnat
/usr/bin/sdnatd
/usr/include/sdnat.h
/usr/lib/libsdnat.a
/usr/lib/libsdnat.so*
%dir /usr/share/doc/sdnat
/usr/share/doc/sdnat/*

%changelog

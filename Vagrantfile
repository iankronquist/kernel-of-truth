Vagrant.configure("2") do |c|
  c.vm.box = "centos7"
  c.vm.box_url = "http://opscode-vm-bento.s3.amazonaws.com/vagrant/virtualbox/opscode_centos-7.0_chef-provisionerless.box"
  c.vm.hostname = "kot"
  # c.vm.network(:forwarded_port, {:guest=>8000, :host=>8080})
  c.vm.synced_folder ".", "/home/vagrant/project"
  c.vm.provider :virtualbox do |p|
  end

  c.vm.provision "shell", inline: <<-SHELL
    echo 'export GCC_COLORS="error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01"' >> /home/vagrant/.bashrc
    sudo yum install -y http://pkgs.repoforge.org/rpmforge-release/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm
    sudo yum install -y xorriso gdb nasm llvm grub2-tools
  SHELL

end


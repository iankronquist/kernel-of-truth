Vagrant.configure("2") do |c|
  c.vm.box = "ubuntu"
  c.vm.box_url = "http://opscode-vm-bento.s3.amazonaws.com/vagrant/virtualbox/opscode_ubuntu-14.04_chef-provisionerless.box"
  c.vm.hostname = "kot"
  # c.vm.network(:forwarded_port, {:guest=>8000, :host=>8080})
  c.vm.synced_folder ".", "/home/vagrant/project"
  c.vm.provider :virtualbox do |p|
  end

  c.vm.provision "shell", inline: <<-SHELL
    echo 'export GCC_COLORS="error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01"' >> /home/vagrant/.bashrc
    sudo apt-get update
    sudo apt-get install -y xorriso gdb nasm llvm grub-common qemu
  SHELL

end


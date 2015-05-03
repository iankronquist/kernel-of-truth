Vagrant.configure("2") do |c|
  c.vm.box = "centos7"
  c.vm.box_url = "http://opscode-vm-bento.s3.amazonaws.com/vagrant/virtualbox/opscode_centos-7.0_chef-provisionerless.box"
  c.vm.hostname = "pop"
  # c.vm.network(:forwarded_port, {:guest=>8000, :host=>8080})
  c.vm.synced_folder ".", "/home/vagrant/project"
  c.vm.provider :virtualbox do |p|
  end
end

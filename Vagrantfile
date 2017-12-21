# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
	config.vm.box = "ubuntu/xenial64"
	config.vm.synced_folder ".", "/home/ubuntu/gpuflow"
	config.vm.provider "virtualbox" do |v|
		v.cpus =2
	end
    config.vm.provision "shell", inline: <<-SHELL
 	  apt-get update
	  apt-get install -y make coreutils gcc gcc-multilib libnuma-dev python
    SHELL
end

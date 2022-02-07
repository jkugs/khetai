require "bundler/gem_tasks"
task :default => :spec

require "rake"
require "rake/extensiontask"
Rake::ExtensionTask.new "khetai" do |ext|
  ext.lib_dir = "lib/khetai"
end
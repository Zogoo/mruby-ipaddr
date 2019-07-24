MRuby::Build.new do |conf|
  toolchain :gcc
  enable_debug
  conf.enable_test
  conf.gem File.expand_path(File.dirname(__FILE__))

  case ENV['MRB_INT_SIZE']
  when 'MRB_INT16'
    conf.cc.defines = %w(MRB_INT16)
  when 'MRB_INT32'
    conf.cc.defines = %w(MRB_INT32)
  when 'MRB_INT64'
    conf.cc.defines = %w(MRB_INT64)
  end
end

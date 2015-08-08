# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'multipart_parser/version'

Gem::Specification.new do |spec|
  spec.name          = "multipart_parser"
  spec.version       = MultipartParser::VERSION
  spec.authors       = ["Benjamin Bryant"]
  spec.extensions    = ["ext/multipart_parser/extconf.rb"]
  spec.summary       = %q{multipart/form-data parser}
  spec.description   = %q{Ruby bindings for https://github.com/iafonov/multipart-parser-c}
  spec.homepage      = ""
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "rspec"
end

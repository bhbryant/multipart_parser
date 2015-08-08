require "multipart_parser/version"
require "multipart_parser/multipart_parser"

class MultipartParser

  class << self

    #Multiple headers are listed as arrays
    attr_reader :default_header_value_type 

    def default_header_value_type=(val)
      if (val != :mixed && val != :strings && val != :arrays)
        raise ArgumentError, "Invalid header value type"
      end
      @default_header_value_type = val
    end
  end

end

MultipartParser.default_header_value_type = :mixed
# MultipartParser

A c backed multipart/form-data parser. 

Ruby extenstion for https://github.com/iafonov/multipart-parser-c, modeled after http_parser.rb


Multipart-parser C has been modfied to have a more consistent callback structure with http_parser.rb


## Installation

Add this line to your application's Gemfile:

```ruby
gem 'multipart_parser'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install multipart_parser

## Usage



```ruby

require 'multipart_parser'

parser = MultipartParser.new("Boundary+17376C87E2579930")


# Setups callbacks
parser.on_message_begin = proc { puts "# => on_message_begin"}
parser.on_part_begin = proc { puts "# => on_part_begin"}
parser.on_headers_complete = proc {|headers| puts "# => on_headers_complete: #{headers.inspect}"}
parser.on_data = proc {|data| puts "# => on_data: #{data}"}
parser.on_part_complete = proc {puts "# => on_part_complete"}
parser.on_message_complete = proc {puts "# => on_message_complete"}


parser << "--Boundary+17376C87E2579930\r\n"

# => on_message_begin
# => on_part_begin

parser << "Content-Disposition: form-data; name=ID1\r\n"
parser << "Content-Type: text/plain\r\n\r\n"
parser << "This is the first part.\r\n"

# => on_headers_complete: {"Content-Disposition"=>"form-data; name=ID1", "Content-Type"=>"text/plain"}
# => on_data: This is the first part.



parser << "--Boundary+17376C87E2579930\r\n"

# => on_part_complete
# => on_part_begin
# => 
parser << "Content-Disposition: form-data; name=ID2\r\n"
parser << "Content-Type: text/plain\r\n\r\n"
parser << "This is the second part.\r\n";

parser << "--Boundary+17376C87E2579930--\r\n"

# => on_headers_complete: {"Content-Disposition"=>"form-data; name=ID2", "Content-Type"=>"text/plain"}
# => on_data: This is the second part.
# => on_part_complete
# => on_message_complete

```

```ruby
# Also allows you to pass a callback object


class Handler
  def on_message_begin;  puts "# => on_message_begin"; end
  def on_part_begin;  puts "# => on_part_begin"; end
  def on_headers_complete(headers); puts "# => on_headers_complete: #{headers.inspect}"; end
  def on_data(data); puts "# => on_data: #{data}"; end
  def on_part_complete; puts "# => on_part_complete"; end
  def on_message_complete; puts "# => on_message_complete"; end
end


parser = MultipartParser.new("Boundary+17376C87E2579930", Handler.new)


parser << "--Boundary+17376C87E2579930\r\n"

# => on_message_begin
# => on_part_begin

parser << "Content-Disposition: form-data; name=ID1\r\n"
parser << "Content-Type: text/plain\r\n\r\n"
parser << "This is the first part.\r\n"

# => on_headers_complete: {"Content-Disposition"=>"form-data; name=ID1", "Content-Type"=>"text/plain"}
# => on_data: This is the first part.

parser << "--Boundary+17376C87E2579930\r\n"

# => on_part_complete
# => on_part_begin


parser << "Content-Disposition: form-data; name=ID2\r\n"
parser << "Content-Type: text/plain\r\n\r\n"
parser << "This is the second part.\r\n";

parser << "--Boundary+17376C87E2579930--\r\n"

# => on_headers_complete: {"Content-Disposition"=>"form-data; name=ID2", "Content-Type"=>"text/plain"}
# => on_data: This is the second part.
# => on_part_complete
# => on_message_complete

```

```ruby

# Accepts a config paramter to modify how the headers are returned

parser = MultipartParser.new("Boundary+17376C87E2579930", Handler.new, :arrays)

# valid values are :arrays, :strings, :mixed

parser << "--Boundary+17376C87E2579930\r\n"
parser << "Content-Disposition: form-data; name=ID_A\r\n"
parser << "Content-Disposition: form-data; name=ID_B\r\n"
parser << "Content-Type: text/plain\r\n\r\n"
parser << "This is the first part.\r\n"


# => on_headers_complete: {"Content-Disposition"=>["form-data; name=ID_A", "form-data; name=ID_B"], "Content-Type"=>["text/plain"]}


parser = MultipartParser.new("Boundary+17376C87E2579930", Handler.new, :strings)

# => on_headers_complete: {"Content-Disposition"=>"form-data; name=ID_A, form-data; name=ID_B", "Content-Type"=>"text/plain"}



parser = MultipartParser.new("Boundary+17376C87E2579930", Handler.new, :mixed)

# => on_headers_complete: {"Content-Disposition"=>["form-data; name=ID_A", "form-data; name=ID_B"], "Content-Type"=>"text/plain"}


```


```ruby

# Set the default(:mixed)


MultipartParser.default_header_value_type = :arrays


parser = MultipartParser.new("Boundary+17376C87E2579930", Handler.new)

# => on_headers_complete: {"Content-Disposition"=>["form-data; name=ID_A", "form-data; name=ID_B"], "Content-Type"=>["text/plain"]}


```



## Contributing

1. Fork it ( https://github.com/[my-github-username]/multipart_parser/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request

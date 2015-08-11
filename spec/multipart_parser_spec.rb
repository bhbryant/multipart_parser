require 'spec_helper'

describe MultipartParser do  

  let(:handler) {  proc {} }

  def execute(parser)
    parser << "--Boundary+17376C87E2579930\r\nContent-Disposition: form-data; name=ID; paramName=TEST_PARAM_1\r\nContent-Transfer-Encoding: binary\r\nContent-Type: text/plain\r\n\r\nfirst frame\r\n";
    parser << "--Boundary+17376C87E2579930\r\nContent-Disposition: form-data; name=ID; paramName=TEST_PARAM_2\r\nContent-Transfer-Encoding: binary\r\nContent-Type: text/plain\r\n\r\nsecond frame\r\n";
    parser << "--Boundary+17376C87E2579930--\r\n"
  end



  context "with assigned callbacks" do
    let(:parser) { MultipartParser.new "Boundary+17376C87E2579930" }

    it "emits message_begin" do
      parser.on_message_begin = handler

      expect(handler).to receive(:call).once

      execute(parser)
    end

    it "emits part_begin" do
      parser.on_part_begin = handler

      expect(handler).to receive(:call).twice
      
      execute(parser)
    end

    it "parse multipart headers" do
      parser.on_headers_complete = handler


      expect(handler).to receive(:call).with({"Content-Disposition"=>"form-data; name=ID; paramName=TEST_PARAM_1", "Content-Transfer-Encoding"=>"binary", "Content-Type"=>"text/plain"})
      expect(handler).to receive(:call).with({"Content-Disposition"=>"form-data; name=ID; paramName=TEST_PARAM_2", "Content-Transfer-Encoding"=>"binary", "Content-Type"=>"text/plain"})

      execute(parser)
    end

    it "parses multipart data" do
     
      parser.on_data = handler

      expect(handler).to receive(:call).with("first frame")
      expect(handler).to receive(:call).with("second frame")

      execute(parser)
    end

    it "emits part_complete" do
      parser.on_part_complete = handler

      expect(handler).to receive(:call).twice
      
      execute(parser)
    end

    it "emits message_complete" do
      parser.on_message_complete = handler

      expect(handler).to receive(:call).once

      execute(parser)
    end


  end

  context "with callback object" do
    let(:callback_obj) { double("Callback")}
    let(:parser) { MultipartParser.new "Boundary+17376C87E2579930", callback_obj }


    it "emits message_begin" do
      expect(callback_obj).to receive(:on_message_begin).once

      execute(parser)
    end

    it "emits part_begin" do
      expect(callback_obj).to receive(:on_part_begin).twice
      
      execute(parser)
    end


    it "parse multipart headers" do
      expect(callback_obj).to receive(:on_headers_complete).with({"Content-Disposition"=>"form-data; name=ID; paramName=TEST_PARAM_1", "Content-Transfer-Encoding"=>"binary", "Content-Type"=>"text/plain"})
      expect(callback_obj).to receive(:on_headers_complete).with({"Content-Disposition"=>"form-data; name=ID; paramName=TEST_PARAM_2", "Content-Transfer-Encoding"=>"binary", "Content-Type"=>"text/plain"})

      execute(parser)
    end


    it "parses multipart data" do

      expect(callback_obj).to receive(:on_data).with("first frame")
      expect(callback_obj).to receive(:on_data).with("second frame")

      execute(parser)
    end

    it "emits part_complete" do
      expect(callback_obj).to receive(:on_part_complete).twice
      
      execute(parser)
    end

    it "emits message_complete" do
      expect(callback_obj).to receive(:on_message_complete).once

      execute(parser)
    end

  end

  it "allows the type of the header values to be configured" do
    
    parser = MultipartParser.new("Boundary+17376C87E2579930", nil, :arrays)
    parser.on_headers_complete = handler

    expect(handler).to receive(:call).with({"Content-Disposition"=>["form-data; name=ID_A", "form-data; name=ID_B"], "Content-Type"=>["text/plain"]})

    parser << "--Boundary+17376C87E2579930\r\nContent-Disposition: form-data; name=ID_A\r\nContent-Disposition: form-data; name=ID_B\r\nContent-Type: text/plain\r\n\r\ntext\r\n"
     
  end

  it "allows the default header value type to be set" do
    value_type = MultipartParser.default_header_value_type

    MultipartParser.default_header_value_type = :arrays

    parser = MultipartParser.new("Boundary+17376C87E2579930")


    parser.on_headers_complete = handler

    expect(handler).to receive(:call).with({"Content-Disposition"=>["form-data; name=ID_A", "form-data; name=ID_B"], "Content-Type"=>["text/plain"]})

    parser << "--Boundary+17376C87E2579930\r\nContent-Disposition: form-data; name=ID_A\r\nContent-Disposition: form-data; name=ID_B\r\nContent-Type: text/plain\r\n\r\ntext\r\n"

    ## reset
    MultipartParser.default_header_value_type = value_type
  end

  it 'has a version number' do
    expect(MultipartParser::VERSION).not_to be nil
  end


  specify { expect { MultipartParser.new }.to raise_error(ArgumentError) }

  context "multipart_parser.c" do
    let(:parser) { MultipartParser.new "Boundary+17376C87E2579930" }


    # https://www.ietf.org/rfc/rfc2046.txt

    #   boundary := 0*69<bchars> bcharsnospace

    #   bchars := bcharsnospace / " "

    #   bcharsnospace := DIGIT / ALPHA / "'" / "(" / ")" /
    #                    "+" / "_" / "," / "-" / "." /
    #                    "/" / ":" / "=" / "?"

    # Overall, the body of a "multipart" entity may be specified as
    # follows:

    #   dash-boundary := "--" boundary
    #                    ; boundary taken from the value of
    #                    ; boundary parameter of the
    #                    ; Content-Type field.

    #   multipart-body := [preamble CRLF]
    #                     dash-boundary transport-padding CRLF
    #                     body-part *encapsulation
    #                     close-delimiter transport-padding
    #                     [CRLF epilogue]


    #   transport-padding := *LWSP-char
    #                        ; Composers MUST NOT generate
    #                        ; non-zero length transport
    #                        ; padding, but receivers MUST
    #                        ; be able to handle padding
    #                        ; added by message transports.

    #   encapsulation := delimiter transport-padding
    #                    CRLF body-part

    #   delimiter := CRLF dash-boundary

    #   close-delimiter := delimiter "--"

    #   preamble := discard-text

    #   epilogue := discard-text

    #   discard-text := *(*text CRLF) *text
    #                   ; May be ignored or discarded.

    #   body-part := MIME-part-headers [CRLF *OCTET]
    #                ; Lines in a body-part must not start
    #                ; with the specified dash-boundary and
    #                ; the delimiter must not appear anywhere
    #                ; in the body part.  Note that the
    #                ; semantics of a body-part differ from
    #                ; the semantics of a message, as
    #                ; described in the text.

    #   OCTET := <any 0-255 octet value>

    it "allows a preamble" do
      #https://www.ietf.org/rfc/rfc2046.txt  section 5.1.1

      #    There appears to be room for additional information prior to the
      # first boundary delimiter line and following the final boundary
      # delimiter line.  These areas should generally be left blank, and
      # IMPLEMENTATIONS MUST IGNORE ANYTHING THAT APPEARS BEFORE THE FIRST
      # BOUNDARY DELIMITER LINE OR AFTER THE LAST ONE.

      parser.on_data = handler

      expect(handler).to receive(:call).with("first frame")

      parser << "preamble\r\n--Boundary+17376C87E2579930\r\n\r\nfirst frame\r\n";
      parser << "--Boundary+17376C87E2579930--\r\n"

    end

    it "requires the preamble to be terminated with a CRLF" do

      #   multipart-body := [preamble CRLF]
      #                     dash-boundary transport-padding CRLF

      parser.on_data = handler

      expect(handler).to_not receive(:call)

      parser << "preamble--Boundary+17376C87E2579930\r\n\r\nfirst frame\r\n";
      parser << "--Boundary+17376C87E2579930--\r\n"

    end

    it "handles a preamble that is only CRLF" do

      parser.on_data = handler

      expect(handler).to receive(:call).with("first frame")

      parser << "\r\n--Boundary+17376C87E2579930\r\n\r\nfirst frame\r\n";
      parser << "--Boundary+17376C87E2579930--\r\n"

    end
  end

end


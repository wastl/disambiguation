SRCDIR=../communication

# compile protocol buffers for C
disambiguation_request_pb2.py: $(SRCDIR)/disambiguation_request.proto
	$(PROTOC) -I=$(SRCDIR) --proto_path=. --python_out=. $(SRCDIR)/disambiguation_request.proto



disambiguation_PYTHON = disambiguation_request_pb2.py
disambiguationdir = $(pythondir)/disambiguation

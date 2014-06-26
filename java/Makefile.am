SRCDIR=../communication

# compile protocol buffers for Java
src/main/java/eu/mico/disambiguation/client/DisambiguationProtocol.java: $(SRCDIR)/disambiguation_request.proto
	$(PROTOC) -I=$(SRCDIR) --proto_path=. --java_out=src/main/java $(SRCDIR)/disambiguation_request.proto


all-local: clean-local install-exec-local

clean-local:
	$(MVN) clean
	rm -f src/main/java/eu/mico/disambiguation/client/DisambiguationProtocol.java

install-exec-local: src/main/java/eu/mico/disambiguation/client/DisambiguationProtocol.java
	$(MVN) install -DskipTests


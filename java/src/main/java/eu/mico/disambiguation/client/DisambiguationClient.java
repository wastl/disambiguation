package eu.mico.disambiguation.client;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * This class implements the network interaction with the wsd-disambiguation server. It allows sending
 * DisambiguationRequest elements over the network and wait for the computed result.
 *
 * @author Sebastian Schaffert (sschaffert@apache.org)
 */
public class DisambiguationClient {

    private static Logger log = LoggerFactory.getLogger(DisambiguationClient.class);

    private String host;

    private int port;

    private Socket socket;

    /**
     * Create a new disambiguation client connecting to the given host and port
     * @param host
     * @param port
     */
    public DisambiguationClient(String host, int port) throws IOException {
        this.host = host;
        this.port = port;

        this.socket = new Socket(host,port);
    }


    /**
     * Send a disambiguation request to the server, wait for the result, and return it. Note that this method does not
     * update the original request object but rather creates a new object to return.
     *
     * @param request
     * @return
     */
    public DisambiguationProtocol.DisambiguationRequest sendRequest(DisambiguationProtocol.DisambiguationRequest request) throws IOException {
        OutputStream out = socket.getOutputStream();

        byte[] req_data = request.toByteArray();

        log.info("sending a request of {} bytes ...", req_data.length);

        ByteBuffer data = ByteBuffer.allocate(4 + req_data.length);
        data.order(ByteOrder.BIG_ENDIAN);
        data.putInt(req_data.length);
        data.put(req_data);

        out.write(data.array());
        out.flush();

        log.info("waiting for server response ...");

        InputStream in = socket.getInputStream();

        // read response length from stream (4 byte integer)
        byte[] resp_len = new byte[4];

        int off = 0, len = 4, count;
        while(off < len && ( count = in.read(resp_len,off,len-off)) > 0) {
            off += count;
        }
        len = ByteBuffer.wrap(resp_len).order(ByteOrder.BIG_ENDIAN).getInt();

        log.info("reading a response of {} bytes ...",len);
        byte[] resp_data = new byte[len];

        off = 0;
        while(off < len && ( count = in.read(resp_data,off,len-off)) > 0) {
            off += count;
        }

        return DisambiguationProtocol.DisambiguationRequest.parseFrom(resp_data);
    }



    public void close() throws IOException {
        socket.close();
    }


}

package eu.mico.disambiguation.client.test;

import eu.mico.disambiguation.client.DisambiguationClient;
import eu.mico.disambiguation.client.DisambiguationParser;
import eu.mico.disambiguation.client.DisambiguationProtocol;
import eu.mico.disambiguation.client.DisambiguationWriter;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;

import static eu.mico.disambiguation.client.DisambiguationProtocol.*;

/**
 * Add file description here!
 *
 * @author Sebastian Schaffert (sschaffert@apache.org)
 */
public class DisambiguationClientTest {

    DisambiguationClient client;

    @Before
    public void setup() throws IOException {
        client = new DisambiguationClient("localhost",8888);
    }

    @After
    public void shutdown() throws IOException {
        client.close();
    }


    @Test
    public void testSimple() throws IOException {
        DisambiguationRequest req =
                DisambiguationRequest.newBuilder()
                .addEntities(
                        Entity.newBuilder()
                                .setText("Seattle")
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Seattle").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Seattle_metropolitan_area").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Seattle_Sounders_FC").build())
                        .build()
                )
                .addEntities(
                        Entity.newBuilder()
                                .setText("Washington")
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Washington_(state)").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/George_Washington").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Washington,_D.C.").build())
                                .build()
                )
                .addEntities(
                        Entity.newBuilder()
                                .setText("Portland")
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Portland,_Oregon").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Isle_of_Portland").build())
                                .addCandidates(Candidate.newBuilder().setUri("http://dbpedia.org/resource/Portland,_Maine").build())
                                .build()
                )
                .setMaxdist(3)
                .build();

        DisambiguationRequest resp = client.sendRequest(req);

        System.out.println(resp.toString());
    }

    @Test
    public void testFile() throws IOException {
        String filename = "../test/_N3_Reuters-128_91.cleaned.sug";
        File file = new File(filename);
        DisambiguationRequest req = DisambiguationParser.parseAll(file);

        DisambiguationRequest resp = client.sendRequest(req);

        DisambiguationWriter.writeDisambiguationOrdered(resp);
    }
}

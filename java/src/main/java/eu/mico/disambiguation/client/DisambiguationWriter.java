package eu.mico.disambiguation.client;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import static eu.mico.disambiguation.client.DisambiguationProtocol.*;
import static eu.mico.disambiguation.client.DisambiguationProtocol.Candidate;

/**
 * Add file description here!
 *
 * @author Sebastian Schaffert (sschaffert@apache.org)
 */
public class DisambiguationWriter {


    public static void writeDisambiguationOrdered(DisambiguationRequest req) {
        for(Entity entity : req.getEntitiesList()) {
            System.out.println("Entity: " + entity.getText());

            List<Candidate> candidates = new ArrayList<>(entity.getCandidatesList());
            Collections.sort(candidates, new Comparator<Candidate>() {
                @Override
                public int compare(Candidate candidate, Candidate candidate2) {
                    if(candidate.getConfidence() > candidate2.getConfidence()) {
                        return -1;
                    } else if(candidate.getConfidence() < candidate2.getConfidence()) {
                        return 1;
                    }
                    return 0;
                }
            });

            for(Candidate cand : candidates) {
                System.out.println(" - Candidate: "+cand.getUri()+", confidence "+cand.getConfidence());
            }

        }

    }
}

package eu.mico.disambiguation.client;

import org.apache.commons.io.FileUtils;
import sun.security.pkcs11.wrapper.Constants;

import java.io.File;
import java.io.IOException;
import java.util.Iterator;

import static eu.mico.disambiguation.client.DisambiguationProtocol.*;

/**
 * A simple parser for parsing disambiguation requests stored in special "suggestion" files for the MICO project.
 *
 * @author Sebastian Schaffert (sschaffert@apache.org)
 */
public class DisambiguationParser {


    public static DisambiguationRequest parseAll(File file) throws IOException {
        return parseAll(FileUtils.lineIterator(file));
    }


    /**
	 * Parses a serialized disambiguation request
	 * Works best in combination with {@link FileUtils#lineIterator(java.io.File, String)}
	 * @param lines the iterator over the lines of the files with the annotations
	 * @return the DisambiguationRequest
	 * @throws IllegalStateException if the lines are not well formatted
	 */
	public static DisambiguationRequest parseAll(Iterator<String> lines) {
        DisambiguationRequest.Builder builder = DisambiguationRequest.newBuilder();
        builder.setMaxdist(3);

        Entity entity;
    	while((entity = parse(lines)) != null){
            builder.addEntities(entity);
    	}

        //builder.setCentrality(DisambiguationRequest.CentralityAlgorithm.BETWEENNESS);
        builder.setRelatedness(DisambiguationRequest.RelatednessAlgorithm.SHORTEST_PATH);
        return builder.build();
	}
	/**
	 * Parses a serialized {@link Entity}
	 * Works best in combination with {@link FileUtils#lineIterator(java.io.File, String)}
	 * @param it the iterator over the lines of the files with the annotations
	 * @return the Entity or <code>null</code> if no more lines are provided by the
	 * parsed iterator
	 */
	private static Entity parse(Iterator<String> it){
		String[] selection;
		String entity;
		if(it.hasNext()){
			String line = it.next();
			selection = line.split("\t");
			if(selection.length < 3){
				throw new IllegalStateException("1st line '"+line+"' is not a valid selection!");
			}
		} else {
			return null;
		}
		if(it.hasNext()){
			entity = it.next();
			if(entity.isEmpty()){
				throw new IllegalStateException("2nd line MUST NOT be empty!");
			}
		} else {
			return null;
		}

		//now we can create the annotation
        Entity.Builder eb = Entity.newBuilder();
        eb.setText(selection[2]);

		//try to parse the options
		if(it.hasNext()){
			String options = it.next();
			if(!options.isEmpty()){ //empty line after the entity means that no options are present
				for(String option : options.split("\t")){
                    eb.addCandidates(Candidate.newBuilder().setUri(option).build());
				}
				if(it.hasNext()){
					String empty = it.next();
					if(!empty.isEmpty()){
						throw new IllegalStateException("No empty line after parsed "
								+ selection[2] +"(line: '" + empty + "')!");
					}
				} //else last annotation ... do nothing
			}
		} //else last annotation (without options) ... do nothing
		return eb.build();
	}


}

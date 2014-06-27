import disambiguation_client as wsd

req = wsd.DisambiguationRequest()
req.maxdist = 3
#req.relatedness = wsd.DisambiguationRequest.SHORTEST_PATH
req.centrality = wsd.DisambiguationRequest.PAGERANK


e1 = req.entities.add()
e1.text = "Seattle"

e1c1 = e1.candidates.add()
e1c1.uri = "http://dbpedia.org/resource/Seattle"  

e1c2 = e1.candidates.add()
e1c2.uri = "http://dbpedia.org/resource/Seattle_metropolitan_area" 

e1c3 = e1.candidates.add()
e1c3.uri = "http://dbpedia.org/resource/Seattle_Sounders_FC" 


e2 = req.entities.add()
e2.text = "Washington"

e2c1 = e2.candidates.add()
e2c1.uri = "http://dbpedia.org/resource/Washington_(state)" 

e2c2 = e2.candidates.add()
e2c2.uri = "http://dbpedia.org/resource/George_Washington"

e2c3 = e2.candidates.add()
e2c3.uri = "http://dbpedia.org/resource/Washington,_D.C."


e3 = req.entities.add()
e3.text = "Portland"

e3c1 = e3.candidates.add()
e3c1.uri = "http://dbpedia.org/resource/Portland,_Oregon" 

e3c2 = e3.candidates.add()
e3c2.uri = "http://dbpedia.org/resource/Isle_of_Portland" 

e3c3 = e3.candidates.add()
e3c3.uri = "http://dbpedia.org/resource/Portland,_Maine" 


client = wsd.DisambiguationClient("localhost",8888)

resp = client.sendRequest(req)


print "Response:\n"
print resp

client.close()

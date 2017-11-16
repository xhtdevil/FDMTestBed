//	Created by Wenxuan Mao on 11/12/17

package testcase1;
import java.util.*;
import java.io.*;

public class MPTCP {

	public static void main(String[] args) {
		if (args.length<1 || args.length > 1){
			System.out.println("Usage: 1 argument. Filename of config");
			System.exit(-1);
		}
			
		String fn = args[0].trim();
		File f = new File(fn);
		Scanner sc = null;
		try{
			sc = new Scanner(f);
		}catch(IOException e ){};


		File outF = new File("MPTCPFlowTable.sh");
		PrintWriter pw = null;
		try{
			pw = new PrintWriter(new FileWriter(outF));
		}catch (IOException e){};
		pw.println("#!/bin/bash\n");

		int nship, nsat, numHosts = 0;
		//1st line
		String line = sc.nextLine();
		String[] sp = line.split(" ");
		nship = Integer.parseInt(sp[0]);
		nsat = Integer.parseInt(sp[1]);

		//2nd line
		line = sc.nextLine();
		sp = line.split(" ");

		for (int i = 0; i < sp.length; i++){
			numHosts += Integer.parseInt(sp[i]);
		}
		int hosts [] = new int [numHosts];
		for (int i = 0; i < sp.length; i++){
			hosts[i] = Integer.parseInt(sp[i]);
		}





		//connectivity matrix
		List<LinkedList<Integer>> connect = new ArrayList<LinkedList<Integer>>();
		int satBase = nship+1;
		HashMap<Integer, Integer> hm = new HashMap<Integer,Integer>();
		for (int i = 0; i < nship;){

			line = sc.nextLine();
			if (line.equals(""))
				continue;
			connect.add(new LinkedList<Integer>());
			sp = line.split(" ");
			for (int j = 0; j < sp.length; j++){
				if (sp[j].equals("1")){
					connect.get(i).add(j+satBase);
					if (hm.containsKey(j+satBase)){
						int curr = hm.get(j+satBase);
						hm.put(j+satBase, curr+1);
					}else
						hm.put(j+satBase, 1);
				}
			}
			i++;
		}

		//print s1-s5
		for (int i = 0; i < connect.size();i ++){
			List<Integer> l = connect.get(i);

			for (int j = 0; j < l.size(); j++ ){
				pw.println("sudo ovs-ofctl add-flow s"+(i+1)+" ip,nw_src=10.0." + (i+1)+"."+j +"/32,actions=output:"+(hosts[i]+1+j));
				pw.println("sudo ovs-ofctl add-flow s"+(i+1)+ " in_port="+(hosts[i]+1+j)+",actions=normal");

			}
			pw.println("sudo ovs-ofctl add-flow s"+(i+1)+ " priority=100,actions=normal");

		}
		pw.println();


		//print s6-s8
		int init = nship+1;
		for (Map.Entry<Integer, Integer> entry : hm.entrySet()) {
			//Integer sat = entry.getKey();
			Integer count = entry.getValue();
			
			for (int j = 0 ; j < count; j++)
				pw.println("sudo ovs-ofctl add-flow s"+init+" in_port="+(j+1)+",actions=output:"+(count+1));
			pw.println("sudo ovs-ofctl add-flow s"+init+" in_port="+(count+1)+",actions=normal");
			init++;
		}
		
		
		//print s9-s11
		for (int i = 0 ; i < nsat; i++){
			pw.println("sudo ovs-ofctl add-flow s"+(nship+nsat+1+i)+" in_port=1,actions=output:2");
			pw.println("sudo ovs-ofctl add-flow s"+(nship+nsat+1+i)+" in_port=2,actions=normal");
		}
		//hub
		for (int i = 0 ; i < nsat; i++){
			pw.println("sudo ovs-ofctl add-flow s12 in_port="+(i+1)+" ,actions=output:"+(nsat+1));
		}
		
		



		//bandwidth


		pw.close();
	}

}

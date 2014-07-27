import java.util.List;


public class Simulator {

	public static void simulator (List<Process> processbuffer){
		String test;
		boolean test2;
		for (int i=0;i<processbuffer.size();i++){
			test = processbuffer.get(i).getId();
			for (int j=0;j<processbuffer.get(i).getInstructionSet().size();j++){
				test = processbuffer.get(i).getInstructionSet().get(j).getMessage();
				test2 = processbuffer.get(i).getInstructionSet().get(j).getMutex();
				System.out.print(test);
				System.out.println("   " + test2);
			}
			
		}
	}

}

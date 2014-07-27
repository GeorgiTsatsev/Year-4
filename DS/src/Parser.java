import java.io.BufferedReader;
//import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Parser {

	public static void simulator(List<Process> processbuffer) {

		String id;
		String src_dest;
		String message;
		int lclock = 0;
		List<Message> msgpool = new ArrayList<Message>();
		
		Message temp = new Message();
		Instruction.types type;
		boolean mutex = false;
		Process.states current_state;

		do {

			for (int i = 0; i < processbuffer.size(); i++) {

				if (processbuffer.get(i).getInstructionSet().size() > 0) {

					Instruction instruct = processbuffer.get(i)
							.getInstructionSet().get(0);
					id = processbuffer.get(i).getId();
					type = instruct.getType();
					message = instruct.getMessage();
					lclock = processbuffer.get(i).getClock();

					current_state = processbuffer.get(i).getState();
					src_dest = instruct.getSrcDest();

					switch (type) {

					case send:

						processbuffer.get(i).incClock();
						lclock = processbuffer.get(i).getClock();
					
						System.out.println("sent" + " " + id + " " + message
								+ " " + src_dest + " " + lclock);

						temp = new Message(id, src_dest, message, lclock);
						msgpool.add(temp);

						processbuffer.get(i).getInstructionSet().remove(0);

						break;
					case recv:
						
						for (int j = 0; j < msgpool.size(); j++) {
							temp = msgpool.get(j);
							
							if (temp.contains(src_dest, id, message)) {
								//processbuffer.get(i).setPermission(false);
								lclock = Math.max(msgpool.get(j).getClock(),
										lclock) + 1;

								System.out.println("received" + " " + id + " "
										+ message + " " + src_dest + " "
										+ lclock);

								processbuffer.get(i).getInstructionSet()
										.remove(0);
								processbuffer.get(i).setClock(lclock);

								msgpool.remove(j);
								processbuffer.get(i).setFlag(false);
							} else {
								processbuffer.get(i).setFlag(true);
							}
						}

						break;
					case print:
						
						boolean permission;
						int dclock = 0;
						boolean granted = true;
						current_state = processbuffer.get(i).getState();
						processbuffer.get(i).setState(Process.states.Wanted);

						mutex = instruct.getMutex();
						for (int n = 0; n < processbuffer.size(); n++) {
							lclock = processbuffer.get(i).getClock();
							dclock = processbuffer.get(n).getClock();
							permission = processbuffer.get(n).getPermission();

							
							if ((lclock < dclock || (lclock == dclock && i < n))
									&& i != n) {
								granted = granted && permission;
								
							} 
	
						}

						if (granted) {

							processbuffer.get(i).incClock();
							lclock = processbuffer.get(i).getClock();
							processbuffer.get(i).setState(Process.states.Held);
							processbuffer.get(i).setPermission(false);

							System.out.println("printed" + " " + id + " "
									+ message + " " + lclock);
							processbuffer.get(i).getInstructionSet().remove(0);
							if (!mutex) {
								processbuffer.get(i).setState(
										Process.states.Released);
								processbuffer.get(i).setPermission(true);
							}
						} else {
							break;
						}

					}
				} else {
					processbuffer.remove(i);
				}

			}

		} while (!processbuffer.isEmpty());

	}

	/**
	 * @param args
	 * @throws IOException
	 */

	public static void main(String[] args) throws IOException {

		// the name of the input file is passed as argument
		// when running the programme
		BufferedReader in = new BufferedReader(new FileReader(args[0]));
		String line = null;
		boolean mutex = false;
		Process p = new Process();
		List<Process> processbuffer = new ArrayList<Process>();
		while ((line = in.readLine()) != null) {

			// split the line into an array of words
			String[] arr = line.trim().split("[ ]+");

			// if the first word is “begin”
			if (arr[0].equals("begin")) {
				if (arr[1].equals("process")) {

					p.setId(arr[2]);
				} else if (arr[1].equals("mutex")) {
					mutex = true;
					// after this command all the following instructions
					// should be executed “atomically” - as one.
					// TODO - treat the starting of an atomic block
				}
			} // end of dealing with the begins
				// the end commands
			if (arr[0].equals("end")) {
				if (arr[1].equals("mutex")) {
					mutex = false;
				} else if (arr[1].equals("process")) {
					p.setClock(0);
					p.setPermission(true);
					p.setState(Process.states.Released);
					processbuffer.add(p);
					p = new Process();
				}
			} // end of ends
				// the send instructions
			if (arr[0].equals("send")) {

				// arr[1] is the destination
				// arr[2] is the message
				Instruction instruct = new Instruction("send", arr[1], arr[2],
						mutex);
				p.addInstruction(instruct);
				instruct = null;
			}
			// the receive instructions
			if (arr[0].equals("recv")) {
				// TODO - deal with the receive instruction
				// arr[1] is the sender
				// arr[2] is the message
				Instruction instruct = new Instruction("recv", arr[1], arr[2],
						mutex);
				p.addInstruction(instruct);
				instruct = null;
			}
			// the print instructions
			if (arr[0].equals("print")) {// TODO - deal with the print
											// instruction
				// arr[1] is the payload of the task
				Instruction instruct = new Instruction("print", arr[1], mutex);
				p.addInstruction(instruct);
				instruct = null;
			}

		} // end of while
		in.close();
		simulator(processbuffer);
		// SIMULATOOROROROR();
		// now that the file is parsed, you can put to work your processes.
		// TODO - execute the instructions associated to each process.
	}

}

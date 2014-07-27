
public class Instruction {
	
	public enum types{
		send, recv, print
	}
	
	//public String type;
	public String message;
	public String src_dest;
	public types type;
	public boolean mutex;
	
	public Instruction (String type, String message,boolean mutex){
		this.mutex=mutex;
		this.type = types.print;
		this.message = message;
	}
	public Instruction (String type, String src_dest, String message,boolean mutex){
		this.mutex=mutex;
		if (type.equals("recv")){
			this.type=types.recv;
		} else {
			this.type = types.send;
		}
		
		this.src_dest= src_dest;
		this.message = message;
	}
	
	public types getType(){
		return type;
	}
	public String getSrcDest(){
		return src_dest;
	}
	public String getMessage(){
		return message;
	}
	public boolean getMutex(){
		return mutex;
	}
}

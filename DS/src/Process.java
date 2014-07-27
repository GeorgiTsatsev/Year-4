import java.util.ArrayList;
import java.util.List;


public class Process {
	
	public enum states{
		Released,Wanted,Held
	}
	
	public String id;
	public List<Instruction> instructionset = new ArrayList<Instruction>();
	public int clock;
	public states state;
	public boolean permission;
	public boolean flag;
	
	public void setFlag(boolean flag){
		this.flag=flag;
	}
	public boolean getFlag(){
		return flag;
	}
	public void setPermission(boolean permission){
		this.permission=permission;
	}
	
	public boolean getPermission(){
		return permission;
	}
	
	public void setClock(int clock){
		this.clock = clock;
	}
	public void setState(states state){
		this.state=state;
	}
	public void incClock(){
		clock++;
	}
	public void setId(String id){
		this.id = id;
	}
	public void addInstruction(Instruction instruction){
		instructionset.add(instruction);
	}
	public states getState(){
		return state;
	}
	public int getClock(){
		return clock;
	}
	public String getId(){
		return id;
	}
	public List<Instruction> getInstructionSet(){
		return instructionset;
	}
	
}

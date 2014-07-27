
public class Message {
	int clock;
	String sender;
	String receiver;
	String message;
	
	public Message (){
		
	}
	public Message(String sender, String receiver, String message, int clock){
		this.clock = clock;
		this.sender = sender;
		this.receiver = receiver;
		this.message = message;
	}

	public int getClock(){
		return clock;
	}
	public String getMessage(){
		return message;
	}
	public String getSender(){
		return sender;
	}
	public String getReceiver(){
		return receiver;
	}
	public boolean contains(String sender, String receiver, String message){
		return this.sender.equals(sender)&&this.receiver.equals(receiver)&&this.message.equals(message);
	}
}

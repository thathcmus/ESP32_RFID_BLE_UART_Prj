#define LEDPIN 2
String cmd;
char passwordTemp[10] ="" ;
char password[10] = "abcdef";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LEDPIN,OUTPUT);
}

void loop() {
  Serial.println("=================================================================");
  Serial.println("\t\t1.Type password to open your door");
  Serial.println("\t\t2.Change your password");
  Serial.println("\t\t3.Close your door");
  Serial.print("\t\tEnter your choose:");
  while (Serial.available() == 0) {}     //wait for data available
  cmd = Serial.readString();
  cmd.trim(); 
  Serial.println(cmd);                     g


  if(cmd == "1")
  {
    getPassToOPenDoor();
  }
  else if(cmd == "2")
  {
    changePass();
  }else if(cmd == "3")
  {
    closeDoor();
  }
 
}

void convertCharToString(){

}

void getPassToOPenDoor() {
  Serial.println("=================================================================");
  Serial.print("Type Your Password:");
  for(int i = 1; i<= 3; i++)
    {
      while (Serial.available() == 0) {}     //wait for data available
      cmd = Serial.readString(); 
      cmd.trim();  
      cmd.toCharArray(passwordTemp,cmd.length() + 1);
      Serial.println(passwordTemp);
      if(strcmp(password,passwordTemp)){
          Serial.print("\t\t\tWRONG PASSWORD: ");
          Serial.println(i);
          if(i<3)
          {
          Serial.print("Type Your Password Again:");
          }else{
            Serial.println("=================================================================");
            Serial.println("\t\t\tWARNING!!!");
          }
      }else{
          digitalWrite(LEDPIN, HIGH);
          Serial.println("\t\t\tYOUR DOOR IS OPENED");
          break;
      }
    }
}

void changePass(){
      Serial.print("Type your old Password: ");
       for(int i = 1;i <= 3; i++){
           while (Serial.available() == 0) {}  
            cmd = Serial.readString(); 
            cmd.trim();
            cmd.toCharArray(passwordTemp,cmd.length() + 1);
            Serial.println(cmd);
            if(strcmp(passwordTemp,password))
              {
                  Serial.print("\t\t\tWRONG PASSWORD: ");
                  Serial.println(i);
                  if(i<3){
                  Serial.println("=================================================================");
                  Serial.print("Type your old Password again: ");
                  }
                  else{
                  Serial.println("=================================================================");
                  Serial.println("\t\t\tWARNING!!!");
                  }
              }
            else{
                  Serial.print("Type your new Password: ");
                  while (Serial.available() == 0) {}  
                  cmd = Serial.readString(); 
                  cmd.trim();
                  cmd.toCharArray(passwordTemp,cmd.length() + 1);
                  Serial.println(passwordTemp);
                  strcpy(password,passwordTemp);
                  Serial.println("\t\tCHANGE YOUR NEW PASSWORD SUCCESS");
                  break;
            }
      }
}

void closeDoor(){
    Serial.println("=================================================================");
    digitalWrite(LEDPIN, LOW);
    Serial.println("\t\t\tYOUR DOOR IS CLOSED");
}
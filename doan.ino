#define LEDPIN 2
String cmd;
String passwordTemp = "abc";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LEDPIN,OUTPUT);
  Serial.println("Hello, ESP32!");
}
void loop() {
  Serial.println("=================================================================");
  Serial.println("\t\t1.Type password to open your door");
  Serial.println("\t\t2.Change your password");
  Serial.println("\t\t3.Close your door");
  Serial.print("\t\tEnter your choose:");
    while (Serial.available() == 0) {}     //wait for data available
  cmd = Serial.readString();  //read until timeout
  cmd.trim(); 
  Serial.println(cmd);                       // remove any \r \n whitespace at the end of the String
  if(cmd == "1")
  {
    Serial.println("=================================================================");
    Serial.print("Type Your Password:");
    for(int i = 1; i<= 3; i++)
    {
      while (Serial.available() == 0) {}     //wait for data available
      cmd = Serial.readString(); 
      cmd.trim();  
      Serial.println(cmd);  
      if(cmd != passwordTemp){
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
  else if(cmd == "2")
  {
      Serial.print("Type your old Password: ");
       for(int i = 1;i <= 3; i++){
           while (Serial.available() == 0) {}  
            cmd = Serial.readString(); 
            cmd.trim();
            Serial.println(cmd);

            if(cmd != passwordTemp)
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
                  Serial.println(cmd);
                  passwordTemp = cmd;
                  Serial.println("\t\tCHANGE YOUR NEW PASSWORD SUCCESS");
                  break;
            }
  }
  }else if(cmd == "3")
  {
    Serial.println("=================================================================");
    digitalWrite(LEDPIN, LOW);
    Serial.println("\t\t\tYOUR DOOR IS CLOSED");
  }

}
#include "HTTPSRedirect.h"
#include "DebugMacros.h"

// The ID below comes from Google Sheets.
// Towards the bottom of this page, it will explain how this can be obtained
const char *GScriptId = "AKfycbyGa1nN5mtPtbpD8fcBM7KdLmjv8_dZltS9CN1UH0UwS0F_pmhg";

const char* host = "script.google.com";

const int httpsPort = 443;
HTTPSRedirect* client;

// Prepare the url (without the varying data)
String url = String("/macros/s/") + GScriptId + "/exec?";

int setup_sheets_connection() {
  client = new HTTPSRedirect(httpsPort); // TODO: Fix memory leak with delete
  client->setInsecure(); 
  client->setPrintResponseBody(true); // Print endpoint responses to Serial
  client->setContentTypeHeader("application/json");

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<20; i++){
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return -1;
  } else {
    Serial.print(String("HTTPSRedirecter connected to "));
    Serial.println(host);
  }

  return 0;
}

// This is the main method where data gets pushed to the Google sheet
void postData(){

  String urlFinal = url + 
    "maxWeight=" + String(maxWeight) +
    "&aveWeight=" + String(aveWeight) +
    "&lastHang=" + String(last_hang_secs) +
    "&person=" + String(person) +
    "&rung=" + String(rung) +
    "&calib=" + String(calib) +
    "&note=" + String(note); // TODO: Escape the string for URL safety

  Serial.println("Connecting to URL: ");
  Serial.println(urlFinal);

  bool getSuccess = client->GET(urlFinal, host);

  if(!getSuccess) {
    Serial.println("HTTPS Get failed");
  }
}

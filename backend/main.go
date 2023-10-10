package main

import (
	"fmt"
	"log"
	"net/http"
	"strings"
	"io/ioutil"
)

func main() {
	http.Handle("/", http.FileServer(http.Dir("./")))
	http.HandleFunc("/send_char", handleChar)

	log.Println("Server running on port: 8080")
	http.ListenAndServe(":8080", nil)
}


func handleChar(w http.ResponseWriter, r *http.Request) {
    log.Println("Received a request on /send_char")  // <-- Add this log

    if r.Method == http.MethodPost {
		defer r.Body.Close()

		bodyBytes, err := ioutil.ReadAll(r.Body)
		if err != nil {
			log.Println("Error reading the body:", err)  // <-- Modify this to print error immediately
			http.Error(w, "Error reading request", http.StatusInternalServerError)
			return
		}
		key := string(bodyBytes)
		key = strings.TrimSpace(key)
		
		log.Println("Parsed key:", key)  // <-- Log the parsed key

		sendChar(key)
		w.Write([]byte("OK"))
	} else {
	    log.Println("Received non-POST request on /send_char")  // <-- Add this log for clarity
	}
}


func sendChar(key string) {
	// This is where you handle the received character
	fmt.Println("Received char:", key)
}


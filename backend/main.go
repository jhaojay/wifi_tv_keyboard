package main

import (
	"net/http"
	"os/exec"
	"log"
	"encoding/json"
)

type Command struct {
	Command string `json:"command"`
}

func sendCommandHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == http.MethodPost {
		var cmdData Command
		err := json.NewDecoder(r.Body).Decode(&cmdData)
		if err != nil {
			http.Error(w, "Invalid request", http.StatusBadRequest)
			return
		}

		cmd := exec.Command("mosquitto_pub", "-h", "localhost", "-t", "esp32s3", "-m", cmdData.Command)
		err = cmd.Run()
		if err != nil {
			log.Printf("Failed to execute command: %v", err)
			http.Error(w, "Failed to execute command", http.StatusInternalServerError)
			return
		}

		w.WriteHeader(http.StatusOK)
	}
}

func main() {
	http.Handle("/", http.FileServer(http.Dir("../frontend/")))
	http.HandleFunc("/send-command", sendCommandHandler)

	log.Println("Server started at :8080")
	http.ListenAndServe(":8080", nil)
}


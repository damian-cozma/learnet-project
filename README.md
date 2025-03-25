# Computer Networks Learning Platform Project

This repository contains a client-server system designed to facilitate learning of core computer network concepts. The platform features a graphical client interface for searching information, participating in topic-based discussions, and managing friend lists, while the server handles data management and real-time communication.

## Features

- **Login System:** Authenticate users via username/password stored in a demonstrative login.config file.
  <br>
  <br>
  <img width="998" alt="login" src="https://github.com/user-attachments/assets/280f7d72-7a7a-4df1-90ba-76b7bb9e31ce" />
  <br>
  <br>
- **Menu:** Redirects to the Dictionary/Forum.
  <br>
  <br>
  <img width="998" alt="menu" src="https://github.com/user-attachments/assets/725401c7-5f76-4cc2-b51b-096cc4688b0d" />
  <br>
  <br>
- **Network Concept Search:** ASCII-based dictionary returning detailed explanations for keywords (e.g., "TCP", "DNS").
  <br>
  <br>
  <img width="998" alt="dictionary" src="https://github.com/user-attachments/assets/15845d9f-f649-40a1-8b85-7408fce79f72" />
  <br>
  <br>
  -  Use "help" to get suggested terms.
  <br>
  <img width="998" alt="dictionary_help" src="https://github.com/user-attachments/assets/8dc303ff-bb8a-4cdf-a7b9-b848288687f0" />
  <br>
  <br>
- **Interactive Forum:**
  - Topic Management: Users can initiate new discussions and contribute to existing threads
  <br>
  <img width="999" alt="discussions" src="https://github.com/user-attachments/assets/c56afd0e-c481-4bf1-b337-49bf07e37ebe" />
  <br>
  <br>
  - Real-Time Discussions: Create/join network-related topics with live message updates (3-second refresh)
  <br>
  <br>
  <img width="998" alt="random_disc" src="https://github.com/user-attachments/assets/97f59997-01fa-469f-a287-16a7161a74fa" />
  <br>
  <br>
  - Bookmark System: Save important discussions for quick access
  <br>
  <br>
  <img width="998" alt="bookmarks" src="https://github.com/user-attachments/assets/4f47292f-ea13-4a45-88b9-147be30bfa53" />
  <br>
  <br>
- **Social Features:**
  - Friend List: View/remove friends with X button functionality
  <br>
  <img width="999" alt="friends" src="https://github.com/user-attachments/assets/890f0e64-9141-4113-893c-e47394c82a8c" />
  <br>
  <br>
  - Friend Requests: Send/accept connection invitations between users
  <br>
  <br>
  <img width="999" alt="friendAdd" src="https://github.com/user-attachments/assets/c2b23354-bb83-4917-9a75-3b5d28d2bc3c" />
  <br>
  <br>

## Purpose

Developed as a university project, LearNet demonstrates my ability to create complex network applications while fulfilling these academic requirements:
- Client-server architecture implementation
- Socket programming mastery
- Multi-window GTK interface design
- Real-time communication handling
- File-based data persistence

## Technologies

### Client
- **Core:** C programming language
- **GUI:** GTK3 with CSS styling
- **Networking:** BSD sockets (TCP/IP)
- **OS Compatibility:** UNIX systems (Linux/macOS)

### Server
- **Communication:** Custom protocol over TCP sockets
- **Data Storage:** File-based system for:
  - User credentials (login.config)
  - Discussion threads
  - Friend relationships
  - Bookmark data
- **Concurrency:** Handles multiple clients simultaneously (exact implementation depends on server code)

## User Interface (Romanian)

The interface uses Romanian labels but remains intuitive for non-speakers:
- "Cautare informatii" = Information Search
- "Discutii" = Discussions
- "Lista prieteni" = Friends List
- "Adaugă prieteni" = Add Friends
- "Înapoi" = Back

# Computer Networks Learning Platform Project

This repository contains a client-server system designed to facilitate learning of core computer network concepts. The platform features a graphical client interface for searching information, participating in topic-based discussions, and managing friend lists, while the server handles data management and real-time communication.

## Features

- **Login System:** Authenticate users via username/password stored in a demonstrative login.config file.
  <br>
  <br>
  <img width="998" alt="login" src="https://github.com/user-attachments/assets/5157d314-1155-47bd-ad61-fc301c641d18" />
  <br>
  <br>
- **Menu:** Redirects to the Dictionary/Forum.
  <br>
  <br>
  <img width="998" alt="menu" src="https://github.com/user-attachments/assets/d5a6ef17-6992-4a06-bb27-1ee22fdf8947" />
  <br>
  <br>
- **Network Concept Search:** ASCII-based dictionary returning detailed explanations for keywords (e.g., "TCP", "DNS").
  <br>
  <br>
  <img width="998" alt="dictionary" src="https://github.com/user-attachments/assets/2ed8e524-a159-45c3-a7d0-2a852ea783b1" />
  <br>
  <br>
  -  Use "help" to get suggested terms.
  <br>
  <img width="998" alt="dictionary_help" src="https://github.com/user-attachments/assets/be5f65d8-fabb-4da3-9e98-d842ba179d84" />
  <br>
  <br>
- **Interactive Forum:**
  - Topic Management: Users can initiate new discussions and contribute to existing threads
  <br>
  <img width="999" alt="discussions" src="https://github.com/user-attachments/assets/448560d9-7fe0-41d3-b4fc-528f1619b42d" />
  <br>
  <br>
  - Real-Time Discussions: Create/join network-related topics with live message updates (3-second refresh)
  <br>
  <br>
  <img width="998" alt="random_disc" src="https://github.com/user-attachments/assets/c8a7286e-e9bc-4475-801b-1ecdd612766c" />
  <br>
  <br>
  - Bookmark System: Save important discussions for quick access
  <br>
  <br>
  <img width="998" alt="bookmarks" src="https://github.com/user-attachments/assets/5a46bed8-61b2-422d-bce5-35ac9a66fe5c" />
  <br>
  <br>
- **Social Features:**
  - Friend List: View/remove friends with X button functionality
  <br>
  <img width="999" alt="friends" src="https://github.com/user-attachments/assets/2dfffd9e-8256-41ab-89c3-44cc5d9bd997" />
  <br>
  <br>
  - Friend Requests: Send/accept connection invitations between users
  <br>
  <br>
  <img width="999" alt="friendAdd" src="https://github.com/user-attachments/assets/261f4c35-b5a8-4c2f-8747-861d7d8d8b0a" />
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

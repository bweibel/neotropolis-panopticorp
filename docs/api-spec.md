# API Specification

## Overview
Backend API for game state management and player progression.

## Endpoints

### Authentication
- `POST /api/auth/login` - Player login
- `POST /api/auth/logout` - Player logout
- `GET /api/auth/session` - Check session status

### Game State
- `GET /api/game/state` - Get current player state
- `POST /api/game/state` - Update player state
- `GET /api/game/progress` - Get progression data

### Minigames
- `POST /api/minigame/:type/start` - Start a minigame
- `POST /api/minigame/:type/complete` - Complete a minigame
- `GET /api/minigame/:type/leaderboard` - Get scores

### Messages
- `GET /api/messages` - Get unlocked messages
- `POST /api/messages/:id/read` - Mark message as read

[Full specification pending - see Issue #22]

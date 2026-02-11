# Deployment

## Architecture

```
Internet → Caddy (SSL termination) → Frontend (nginx) / Backend (Express)
                                              ↓
                                         SQLite (volume)
```

## First-time VPS Setup

1. **Create deploy user and directory**
   ```bash
   sudo adduser --disabled-password deploy
   sudo usermod -aG docker deploy
   sudo mkdir -p /opt/panopticorp
   sudo chown deploy:deploy /opt/panopticorp
   ```

2. **Log in to GitHub Container Registry**
   ```bash
   # Create a PAT with read:packages scope at https://github.com/settings/tokens
   echo "YOUR_PAT" | docker login ghcr.io -u bweibel --password-stdin
   ```

3. **Create production .env**
   ```bash
   cd /opt/panopticorp
   cp .env.example .env
   # Edit .env and set SESSION_SECRET
   openssl rand -hex 32  # Use this output for SESSION_SECRET
   ```

4. **Set up GitHub repository secrets**
   - `VPS_HOST`: panopticorp.online
   - `VPS_USER`: deploy
   - `VPS_SSH_KEY`: Private SSH key for deploy user

5. **Create GitHub environment**
   - Go to Settings → Environments → New environment
   - Name: `production`

## Manual Deploy

If needed, you can deploy manually from the VPS:

```bash
cd /opt/panopticorp
docker compose -f docker-compose.prod.yml pull
docker compose -f docker-compose.prod.yml up -d
```

## View Logs

```bash
# All services
docker compose -f docker-compose.prod.yml logs -f

# Specific service
docker compose -f docker-compose.prod.yml logs -f backend
```

## SSL

Caddy automatically provisions and renews SSL certificates from Let's Encrypt. No manual configuration required - just ensure:
- Domain DNS points to VPS IP
- Ports 80 and 443 are open

## Backup

SQLite database is stored in a Docker volume. To backup:

```bash
docker compose -f docker-compose.prod.yml exec backend cat /app/data/panopticorp.db > backup.db
```

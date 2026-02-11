# Build stage
FROM node:20-alpine AS builder

WORKDIR /app

# Install pnpm
RUN corepack enable && corepack prepare pnpm@latest --activate

# Copy workspace files
COPY pnpm-workspace.yaml package.json pnpm-lock.yaml ./
COPY web/shared/package.json ./web/shared/
COPY web/frontend/package.json ./web/frontend/

# Install dependencies
RUN pnpm install --frozen-lockfile

# Copy source
COPY web/shared/ ./web/shared/
COPY web/frontend/ ./web/frontend/

# Build shared types first, then frontend
RUN pnpm --filter @panopticorp/shared build
RUN pnpm --filter @panopticorp/frontend build

# Production stage
FROM nginx:alpine

# Copy built assets
COPY --from=builder /app/web/frontend/dist /usr/share/nginx/html

# Copy nginx config
COPY web/frontend/nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]

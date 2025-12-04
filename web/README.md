# Geocoding System - Web Frontend

A simple, beautiful web interface to test your geocoding system.

## Features

✨ **Clean, Modern UI** - Beautiful gradient design with smooth animations
🔍 **Real-time Search** - Search addresses across distributed data nodes
📊 **Result Ranking** - See relevance scores for each result
🗺️ **Map Integration** - Click to view locations on Google Maps
📱 **Responsive Design** - Works on desktop, tablet, and mobile
⚡ **Fast & Lightweight** - Single HTML file, no build process needed

## Quick Start

### Option 1: Open Directly in Browser

Simply open the file in your browser:

```bash
# macOS
open web/index.html

# Linux
xdg-open web/index.html

# Windows
start web/index.html
```

### Option 2: Serve with Python

For better CORS handling:

```bash
# Python 3
cd web
python3 -m http.server 8000

# Then open: http://localhost:8000
```

### Option 3: Serve with Node.js

```bash
# Install http-server globally
npm install -g http-server

# Serve the web frontend
cd web
http-server -p 8000

# Then open: http://localhost:8000
```

## Prerequisites

Make sure your geocoding system is running:

```bash
# Start the system
docker-compose up -d

# Check health
curl http://localhost:18080/health
```

Expected response: `{"status":"healthy","data_nodes":2}`

## Usage

1. **Open the portal** in your browser
2. **Enter a search query** (e.g., "Salinas", "MCKINNON", "93906")
3. **Click Search** or press Enter
4. **View results** with addresses, coordinates, and relevance scores
5. **Click "View on Map"** to see the location on Google Maps

### Example Searches

The portal includes quick-access chips for common searches:

- **Salinas** - Search by city name
- **MCKINNON** - Search by street name
- **93906** - Search by postal code
- **ARNHEM ROAD** - Search by full street name
- **Seaside** - Search by another city

## Features Explained

### Status Bar
- **Green dot**: Gateway is healthy
- **Red dot**: Gateway is offline or error occurred
- **Node status**: Shows how many data nodes responded

### Search Results
- **Address**: Full formatted address
- **Score**: Relevance score (higher = more relevant)
- **Coordinates**: Latitude and longitude
- **Shard**: Which data node returned this result
- **Map link**: Opens location in Google Maps

### Error Handling
- Shows clear error messages if gateway is offline
- Provides troubleshooting steps
- Displays connection status

## Customization

### Change Gateway URL

Edit the `API_BASE_URL` in the HTML file:

```javascript
const API_BASE_URL = 'http://your-gateway-url:18080';
```

### Change Color Scheme

Modify the CSS gradient in the `<style>` section:

```css
background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
```

### Add More Example Searches

Add more chips in the HTML:

```html
<div class="example-chip" data-query="Your Query">Your Label</div>
```

## Troubleshooting

### "Cannot connect to gateway"

**Check if gateway is running:**
```bash
docker-compose ps
curl http://localhost:18080/health
```

**View gateway logs:**
```bash
docker-compose logs gateway
```

### CORS Errors

If you see CORS errors in the browser console:

1. **Serve the HTML file** using Python or Node.js (see Quick Start)
2. **Or add CORS headers** to your gateway (not recommended for production)

### No Results Found

Make sure you're searching for addresses that exist in your data:

```bash
# Check what's in your data
head -20 data/shard_0_data_demo.csv

# Find street names
cut -d',' -f4 data/shard_0_data_demo.csv | sort -u | head -20
```

## Architecture

```
┌─────────────┐
│   Browser   │
│     (Web)   │
└──────┬──────┘
       │ HTTP POST /api/findAddress
       │ {"address": "Salinas"}
       ▼
┌─────────────┐
│   Gateway   │
│  Port 18080 │
└──────┬──────┘
       │ gRPC Search
       ├──────────┬──────────┐
       ▼          ▼          ▼
┌──────────┐ ┌──────────┐
│ Data     │ │ Data     │
│ Node 0   │ │ Node 1   │
│ Port     │ │ Port     │
│ 50051    │ │ 50052    │
└──────────┘ └──────────┘
```

## Tech Stack

- **HTML5** - Structure
- **CSS3** - Styling with gradients and animations
- **Vanilla JavaScript** - No frameworks needed
- **Fetch API** - HTTP requests to gateway

## Browser Support

- ✅ Chrome/Edge (latest)
- ✅ Firefox (latest)
- ✅ Safari (latest)
- ✅ Mobile browsers

## Performance

- **Lightweight**: Single 15KB HTML file
- **Fast**: No build process or dependencies
- **Efficient**: Minimal JavaScript, no frameworks

## Screenshots

### Search Interface
Clean, modern search interface with example queries

### Results Display
Beautiful result cards with all address details

### Mobile Responsive
Works perfectly on mobile devices

## Next Steps

- Add autocomplete suggestions
- Add search history
- Add result export (CSV/JSON)
- Add map view with multiple markers
- Add advanced filters (city, postal code, etc.)

## License

Part of the Geocoding System project.

---

**Enjoy testing your geocoding system!** 🚀

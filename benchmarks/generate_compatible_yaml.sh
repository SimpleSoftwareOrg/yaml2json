#!/bin/bash

set -e

echo "Generating YAML files compatible with yaml2json, yq, and lq..."

# Clean up old files
rm -f benchmarks/*.yaml

# Small file (~25KB) - basic structures only
cat > benchmarks/small_117kb.yaml << 'EOF'
# Small YAML file for performance testing
# Compatible with yaml2json, yq, and lq
application:
  name: "yaml2json-benchmark"
  version: "1.0.0"
  description: "Performance testing for YAML to JSON conversion tools"
  created: "2024-06-19T17:30:00Z"
  
database:
  host: "localhost"
  port: 5432
  ssl: true
  timeout: 30
  pool_size: 10
  retry_attempts: 3

api:
  version: "v1"
  rate_limit: 1000
  cors_enabled: true
  auth_required: true
  cache_ttl: 3600

users:
EOF

# Generate users data
for i in {1..300}; do
  active_val=$( [ $((i % 2)) -eq 0 ] && echo "true" || echo "false" )
  theme_options=("dark" "light" "auto")
  lang_options=("en" "es" "fr" "de")
  theme=${theme_options[$((i % 3))]}
  language=${lang_options[$((i % 4))]}
  
  cat >> benchmarks/small_117kb.yaml << EOF
  - id: $((1000 + i))
    name: "User ${i}"
    email: "user${i}@benchmark.test"
    age: $((18 + (i % 60)))
    active: ${active_val}
    score: $((i % 100)).$((i % 100))
    created_at: "2024-06-$((10 + (i % 19)))T$(printf "%02d" $((i % 24))):$(printf "%02d" $((i % 60))):$(printf "%02d" $((i % 60)))Z"
    bio: "User ${i} biography with performance testing data. This contains multiple lines of text to simulate real-world YAML documents."
    preferences:
      theme: "${theme}"
      notifications: ${active_val}
      language: "${language}"
      timezone: "UTC"
EOF
done

echo "Generated small_117kb.yaml ($(stat -f%z benchmarks/small_117kb.yaml 2>/dev/null || stat -c%s benchmarks/small_117kb.yaml) bytes)"

# Medium file (~500KB)
cp benchmarks/small_117kb.yaml benchmarks/medium_1mb.yaml

# Add services data
cat >> benchmarks/medium_1mb.yaml << 'EOF'

services:
EOF

for batch in {1..30}; do
  for i in {1..50}; do
    enabled_val=$( [ $((i % 2)) -eq 0 ] && echo "true" || echo "false" )
    caching_val=$( [ $((i % 3)) -eq 0 ] && echo "true" || echo "false" )
    levels=("debug" "info" "warn" "error")
    log_level=${levels[$((i % 4))]}
    
    cat >> benchmarks/medium_1mb.yaml << EOF
  - id: "service_${batch}_${i}"
    name: "Service ${batch}-${i}"
    type: "microservice"
    enabled: ${enabled_val}
    timeout: $((30 + (i % 300)))
    retries: $((1 + (i % 10)))
    endpoints:
      primary: "https://api${i}.example.com"
      backup: "https://backup${i}.example.com"
    configuration:
      user_agent: "BenchmarkClient/1.0"
      accept: "application/json"
      auth_token: "token_${batch}_${i}_benchmark"
      caching: ${caching_val}
      compression: ${enabled_val}
      log_level: "${log_level}"
    metadata:
      created: "2024-06-$((10 + (i % 19)))"
      version: "1.$((i % 10)).$((i % 100))"
      region: "us-west-$((1 + (i % 3)))"
      tags:
        - "performance"
        - "test"
        - "batch${batch}"
        - "service"
EOF
  done
done

echo "Generated medium_1mb.yaml ($(stat -f%z benchmarks/medium_1mb.yaml 2>/dev/null || stat -c%s benchmarks/medium_1mb.yaml) bytes)"

# Large file (~3MB)
cp benchmarks/medium_1mb.yaml benchmarks/large_6_5mb.yaml

# Add events data
cat >> benchmarks/large_6_5mb.yaml << 'EOF'

events:
EOF

for batch in {1..100}; do
  for i in {1..80}; do
    event_types=("user_action" "system_event" "api_call" "database_query" "cache_miss" "auth_event")
    levels=("INFO" "WARN" "ERROR" "DEBUG")
    statuses=("success" "failure" "timeout" "retry")
    
    event_type=${event_types[$((i % 6))]}
    level=${levels[$((i % 4))]}
    status=${statuses[$((i % 4))]}
    
    cat >> benchmarks/large_6_5mb.yaml << EOF
  - event_id: "evt_${batch}_${i}"
    timestamp: "2024-06-$((10 + (batch % 19)))T$(printf "%02d" $((10 + (i % 14)))):$(printf "%02d" $((i % 60))):$(printf "%02d" $((i % 60)))Z"
    type: "${event_type}"
    level: "${level}"
    status: "${status}"
    service: "service_$((1 + (i % 100)))"
    user_id: $((10000 + (i % 100000)))
    session_id: "session_${batch}_${i}"
    request_id: "req_${batch}_${i}_benchmark"
    duration_ms: $((10 + (i % 5000)))
    message: "Event ${i} in batch ${batch}: ${event_type} completed with status ${status}. Performance benchmark data entry."
    details:
      method: "POST"
      endpoint: "/api/v1/action$((i % 50))"
      status_code: $((200 + (i % 299)))
      response_size: $((100 + (i % 10000)))
      client_ip: "$((192 + (i % 63))).$((168 + (i % 87))).$((1 + (i % 254))).$((1 + (i % 254)))"
      user_agent: "BenchmarkClient/$((1 + (i % 10))).$((i % 20))"
      referrer: "https://benchmark$((i % 100)).test.com"
    payload:
      data_size: $((i % 1000))
      compression: "gzip"
      encoding: "utf-8"
      checksum: "sha256_${batch}_${i}"
EOF
  done
done

echo "Generated large_6_5mb.yaml ($(stat -f%z benchmarks/large_6_5mb.yaml 2>/dev/null || stat -c%s benchmarks/large_6_5mb.yaml) bytes)"

# Very large file (~10MB)
cp benchmarks/large_6_5mb.yaml benchmarks/very_large_13mb.yaml

# Add transaction data
cat >> benchmarks/very_large_13mb.yaml << 'EOF'

transactions:
EOF

for batch in {1..150}; do
  for i in {1..60}; do
    currencies=("USD" "EUR" "GBP" "JPY" "CAD" "AUD")
    payment_methods=("credit_card" "bank_transfer" "paypal" "stripe" "crypto")
    transaction_statuses=("pending" "completed" "failed" "cancelled" "refunded")
    countries=("US" "CA" "UK" "DE" "FR" "JP" "AU")
    
    currency=${currencies[$((i % 6))]}
    payment_method=${payment_methods[$((i % 5))]}
    transaction_status=${transaction_statuses[$((i % 5))]}
    country=${countries[$((i % 7))]}
    
    cat >> benchmarks/very_large_13mb.yaml << EOF
  - transaction_id: "txn_${batch}_${i}"
    user_id: $((10000 + (i % 100000)))
    merchant_id: "merchant_$((1 + (i % 1000)))"
    amount: $((1 + (i % 10000))).$((i % 100))
    currency: "${currency}"
    status: "${transaction_status}"
    payment_method: "${payment_method}"
    timestamp: "2024-06-$((10 + (batch % 19)))T$(printf "%02d" $((10 + (i % 14)))):$(printf "%02d" $((i % 60))):$(printf "%02d" $((i % 60)))Z"
    processing_time_ms: $((50 + (i % 5000)))
    risk_score: $((i % 1000))
    metadata:
      gateway: "gateway_$((1 + (i % 10)))"
      processor: "processor_$((1 + (i % 5)))"
      merchant_category: "category_$((1 + (i % 20)))"
      country: "${country}"
      region: "region_$((1 + (i % 50)))"
      city: "city_$((1 + (i % 100)))"
      postal_code: "$(printf "%05d" $((i % 100000)))"
      ip_address: "$((1 + (i % 254))).$((1 + (i % 254))).$((1 + (i % 254))).$((1 + (i % 254)))"
    fees:
      processing_fee: $((i % 100)).$((i % 100))
      currency_conversion: $((i % 50)).$((i % 100))
      total_fees: $((i % 150)).$((i % 100))
    audit:
      created_by: "system"
      updated_by: "system"
      created_at: "2024-06-$((10 + (batch % 19)))T$(printf "%02d" $((10 + (i % 14)))):$(printf "%02d" $((i % 60))):$(printf "%02d" $((i % 60)))Z"
      notes: "Benchmark transaction ${i} in batch ${batch} for performance testing purposes."
EOF
  done
done

echo "Generated very_large_13mb.yaml ($(stat -f%z benchmarks/very_large_13mb.yaml 2>/dev/null || stat -c%s benchmarks/very_large_13mb.yaml) bytes)"

echo ""
echo "All benchmark YAML files generated successfully!"
echo "File sizes:"
for file in benchmarks/*.yaml; do
  size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file")
  size_kb=$((size / 1024))
  size_mb=$((size_kb / 1024))
  if [ $size_mb -gt 0 ]; then
    echo "  $(basename "$file"): ${size_mb}.$(((size_kb % 1024) / 100))MB (${size} bytes)"
  else
    echo "  $(basename "$file"): ${size_kb}KB (${size} bytes)"
  fi
done 
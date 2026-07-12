const { redisClient, DEFAULT_EXPIRATION } = require('../config/redis.config');

// SỬA TẠI ĐÂY: Chuyển thành middleware function trực tiếp
const getDataFromCache = async (req, res, next) => {
    // Lưu ý: in cả object 'res' ra console sẽ làm ngập terminal và chậm ứng dụng, 
    // nên in req.originalUrl sẽ trực quan hơn.
    console.log("I am alive! The request came here was: ", req.originalUrl);
    
    if (req.method !== 'GET') {
        return next();
    } 

    // Tạo cache key tối ưu hơn sử dụng req.originalUrl (bao gồm cả baseUrl + path + query string ban đầu)
    const cacheKey = `cache:${req.originalUrl}`;
    console.log("Key generated: ", cacheKey);

    try {
        const data = await redisClient.get(cacheKey);

        if (data) {
            console.log("Found data in cache, sending back:", data);
            // Sử dụng res.send thay vì res.json(JSON.parse) để tránh việc phải 
            // parse đi parse lại chuỗi JSON đã được lưu trong Redis.
            res.setHeader('Content-Type', 'application/json');
            return res.status(200).send(data);
        } 

        // Khi cache miss, ghi đè res.send để bắt dữ liệu từ DB và lưu vào Redis
        console.log("Cache miss");
        
        const originalSend = res.send;
        res.send = function (body) {
            if (res.statusCode === 200) {
                console.log("Saving data from DB to Redis...");
                
                // Đảm bảo body là string trước khi nạp vào Redis
                const chunk = typeof body === 'object' ? JSON.stringify(body) : body;

                redisClient.setEx(cacheKey, DEFAULT_EXPIRATION, chunk).catch(err => 
                    console.error('Redis save error:', err)
                );
            }
            originalSend.call(this, body);
        };

        // Quan trọng: Phải gọi next() để request tiếp tục đi đến Controller/DB
        next();

    } catch (error) {
        console.error("Redis error: ", error);
        // Nếu Redis lỗi, lập tức bỏ qua và gọi tiếp vào DB (Fail-safe)
        next();
    }
};

module.exports = getDataFromCache;
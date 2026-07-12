const redisClient = require('../config/redis.config');

const DEFAULT_TTL = parseInt(process.env.REDIS_CACHE_TTL) || 300; // Mặc định 5 phút

const redisService = {
    // Đảm bảo client đã kết nối
    connect: async () => {
        if (!redisClient.isOpen) {
            await redisClient.connect();
        }
    },

    // Ngắt kết nối
    disconnect: async () => {
        if (redisClient.isOpen) {
            await redisClient.disconnect();
        }
    },

    // Lấy dữ liệu từ Cache và tự động parse JSON
    get: async (key) => {
        try {
            const data = await redisClient.get(key);
            return data ? JSON.parse(data) : null;
        } catch (error) {
            console.error(`❌ Lỗi Redis GET key "${key}":`, error);
            return null;
        }
    },

    // Lưu dữ liệu vào Cache dưới dạng chuỗi JSON
    set: async (key, value, ttl = DEFAULT_TTL) => {
        try {
            const stringValue = JSON.stringify(value);
            if (ttl) {
                await redisClient.set(key, stringValue, {
                    EX: ttl
                });
            } else {
                await redisClient.set(key, stringValue);
            }
            return true;
        } catch (error) {
            console.error(`❌ Lỗi Redis SET key "${key}":`, error);
            return false;
        }
    },

    // Xóa Cache theo key
    del: async (key) => {
        try {
            await redisClient.del(key);
            return true;
        } catch (error) {
            console.error(`❌ Lỗi Redis DEL key "${key}":`, error);
            return false;
        }
    }
};

module.exports = redisService;
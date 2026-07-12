'use strict';

module.exports = {
  async up(queryInterface, Sequelize) {
    // 1. Thêm cột deviceId vào bảng sensors trước
    await queryInterface.addColumn('sensors', 'deviceId', {
      type: Sequelize.INTEGER,
      allowNull: true, // Để true nếu có cảm biến chưa thuộc thiết bị nào, hoặc đổi thành false tùy nhu cầu
    });

    // 2. Thêm ràng buộc khóa ngoại cho cột vừa tạo
    await queryInterface.addConstraint('sensors', {
      fields: ['deviceId'],
      type: 'foreign key',
      name: 'fk_sensors_devices',
      references: {
        table: 'devices',
        field: 'id'
      },
      onDelete: 'SET NULL', // Nếu xóa thiết bị, cảm biến vẫn giữ lại nhưng deviceId chuyển về NULL
      onUpdate: 'CASCADE'
    });
  },

  async down(queryInterface, Sequelize) {
    // Xóa khóa ngoại trước, sau đó xóa cột
    await queryInterface.removeConstraint('sensors', 'fk_sensors_devices');
    await queryInterface.removeColumn('sensors', 'deviceId');
  }
};
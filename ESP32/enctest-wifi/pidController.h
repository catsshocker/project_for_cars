class PIDController
{
private:
    double kp;
    double ki;
    double kd;

    double lastErr;
    double errSum;

public:
    PIDController(double p, double i, double d);
    ~PIDController();
    void setKp(double p);
    void setKi(double i);
    void setKd(double d);
    double compute(double setpoint, double actual, double max, double min, double dt);
    void reset();
    double getKp() const;
    double getKi() const;
    double getKd() const;
};

PIDController::PIDController(double p, double i, double d)
    : kp(p), ki(i), kd(d), lastErr(0.0), errSum(0.0)
{
}

PIDController::~PIDController() = default;

void PIDController::setKp(double p) { kp = p; }

void PIDController::setKi(double i) { ki = i; }

void PIDController::setKd(double d) { kd = d; }

double PIDController::getKp() const { return kp; }
double PIDController::getKi() const { return ki; }
double PIDController::getKd() const { return kd; }

void PIDController::reset()
{
    lastErr = 0.0;
    errSum = 0.0;
}

double PIDController::compute(double setpoint, double actual, double max, double min, double dt)
{
    if (dt <= 0.0)
        return 0.0;
    double err = setpoint - actual;
    errSum += err * dt;
    double dErr = (err - lastErr) / dt;

    double output = (kp * err) + (ki * errSum) + (kd * dErr);

    if (output > max)
        output = max;
    else if (output < min)
        output = min;

    lastErr = err;

    return output;
}

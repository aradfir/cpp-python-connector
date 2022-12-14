import numpy as np
import random
from keras.models import Sequential
from keras.layers import Dense, Dropout
from keras.optimizers import Adam

from collections import deque


class DQN:
    def __init__(self, state_shape, output_shape):
        self.state_shape = state_shape
        self.output_shape = output_shape
        self.memory = deque(maxlen=2000)

        self.gamma = 0.85
        self.epsilon = 0.99
        self.epsilon_min = 0.01
        self.epsilon_decay = 0.99
        self.learning_rate = 0.02
        self.tau = .125

        self.model = self.create_model()
        self.target_model = self.create_model()

    def create_model(self):
        model = Sequential()

        model.add(Dense(16, input_dim=self.state_shape, activation="relu"))
        model.add(Dense(4, activation="relu"))
        #model.add(Dense(24, activation="relu"))
        model.add(Dense(self.output_shape))
        model.compile(loss="mean_squared_error",
                      optimizer=Adam(lr=self.learning_rate))
        return model

    def act(self, state):
        self.epsilon *= self.epsilon_decay
        self.epsilon = max(self.epsilon_min, self.epsilon)
        if np.random.random() < self.epsilon:
            print('random')
            return random.randint(0, self.output_shape-1)
        return np.argmax(self.model.predict(state)[0])

    def remember(self, state, action, reward, new_state, done):
        self.memory.append([state, action, reward, new_state, done])

    def replay(self):
        batch_size = 16
        if len(self.memory) < batch_size:
            return

        samples = random.sample(self.memory, batch_size)
        for sample in samples:
            state, action, reward, new_state, done = sample
            target = self.target_model.predict(state)
            if done:
                target[0][action] = reward
            else:
                Q_future = max(self.target_model.predict(new_state)[0])
                target[0][action] = reward + Q_future * self.gamma
            self.model.fit(state, target, epochs=1, verbose=0)

    def target_train(self):
        weights = self.model.get_weights()
        target_weights = self.target_model.get_weights()
        for i in range(len(target_weights)):
            target_weights[i] = weights[i] * self.tau + target_weights[i] * (1 - self.tau)
        self.target_model.set_weights(target_weights)

    def save_model(self, fn):
        self.model.save(fn)


current_state, latest_action = None, None
dqn_agent = DQN(11, 4)

from parse_utils import list_to_numpy,encode_message


def get_action(args):
    global latest_action
    latest_action = dqn_agent.act(current_state)
    answer=np.zeros((1,))
    answer[0]=latest_action

    return encode_message("action",answer)


def init_state(args):
    global current_state
    current_state = list_to_numpy(args)
    return b"ok 1 200"


def get_reward_new_state(args):
    global current_state, latest_action
    r = float(args[0])
    new_state = list_to_numpy(args)
    done = bool(args[1])
    dqn_agent.remember(current_state, latest_action, r, new_state, done)
    dqn_agent.replay()
    dqn_agent.target_train()
    current_state = new_state
    return b"ok 1 200"
# def main():
#     env = gym.make("MountainCar-v0")
#     gamma = 0.9
#     epsilon = .95
#
#     trials = 1000
#     trial_len = 500
#
#     # updateTargetNetwork = 1000
#     dqn_agent = DQN(env=env)
#     steps = []
#     for trial in range(trials):
#         cur_state = env.reset().reshape(1, 2)
#         for step in range(trial_len):
#
#             new_state, reward, done, _ = env.step(action)
#
#             # reward = reward if not done else -20
#             new_state = new_state.reshape(1, 2)
#             dqn_agent.remember(cur_state, action, reward, new_state, done)
#
#             dqn_agent.replay()  # internally iterates default (prediction) model
#             dqn_agent.target_train()  # iterates target model
#
#             cur_state = new_state
#             if done:
#                 break
#             if step >= 199:
#                 print("Failed to complete in trial {}".format(trial))
#                 if step % 10 == 0:
#                     dqn_agent.save_model("trial-{}.model".format(trial))
#         else:
#             print("Completed in {} trials".format(trial))
#             dqn_agent.save_model("success.model")
#             break
#
#
# if __name__ == "__main__":
#     main()
